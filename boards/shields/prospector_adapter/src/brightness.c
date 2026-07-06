#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(als, 4);

/* Bind the DISPLAY's pwm-leds controller SPECIFICALLY, via disp_bl's parent node.
 * This build defines a SECOND pwm-leds node (the keyboard-backlight relay
 * placeholder in prototype_mk1_waveshare.overlay), which made the old
 * DEVICE_DT_GET_ONE(pwm_leds) ambiguous -- it could bind the placeholder (an
 * unconnected pad) instead of the display backlight, so brightness went nowhere. */
static const struct device *pwm_leds_dev = DEVICE_DT_GET(DT_PARENT(DT_NODELABEL(disp_bl)));
#define DISP_BL DT_NODE_CHILD_IDX(DT_NODELABEL(disp_bl))

/* Public: step the DISPLAY backlight (0-100%) for the touch settings screen.
 * Only affects the dongle's own display (disp_bl) -- NOT the keyboard &bl relay,
 * which is a separate PWM channel on the halves. */
static uint8_t touch_brightness = CONFIG_PROSPECTOR_FIXED_BRIGHTNESS;
void prospector_brightness_step(int delta) {
    int b = (int)touch_brightness + delta;
    if (b > 100) { b = 100; }
    if (b < 5) { b = 5; }
    int rc = led_set_brightness(pwm_leds_dev, DISP_BL, (uint8_t)b);
    LOG_INF("display brightness -> %d%% (delta %d, rc %d)", b, delta, rc);
    if (rc == 0) {
        touch_brightness = (uint8_t)b;
    }
}

/* Current display brightness % -- for the touch Settings on-screen readout. */
uint8_t prospector_brightness_get(void) {
    return touch_brightness;
}

#ifdef CONFIG_PROSPECTOR_USE_AMBIENT_LIGHT_SENSOR

static uint8_t current_brightness = 100;

#define SENSOR_MIN      0       // Minimum sensor reading
#define SENSOR_MAX      100   // Maximum sensor reading
#define PWM_MIN         1       // Minimum PWM duty cycle (%) - keep display visible
#define PWM_MAX         100     // Maximum PWM duty cycle (%)

#define FADE_STEP                        1
#define FADE_SLEEP_BRIGHTEN_MS           3
#define FADE_SLEEP_DARKEN_MS             10
#define FADE_THRESHOLD                   10

#define NORMAL_SAMPLE_SLEEP_MS           100

#define BURST_SAMPLE_SLEEP_MS            30
#define BURST_SAMPLE_TIMEOUT             10
#define BURST_SAMPLE_CONSECUTIVE         3

uint8_t map_light_to_pwm(int32_t sensor_reading) {
    // Handle invalid/error readings
    if (sensor_reading < SENSOR_MIN) {
        return PWM_MIN;  // Default to minimum brightness on error
    }

    // Clamp to maximum
    if (sensor_reading > SENSOR_MAX) {
        sensor_reading = SENSOR_MAX;
    }

    // Linear mapping
    uint8_t pwm_value = (uint8_t)(
        PWM_MIN + ((PWM_MAX - PWM_MIN) *
        (sensor_reading - SENSOR_MIN)) / (SENSOR_MAX - SENSOR_MIN)
    );

    return pwm_value;
}

/* Fade the display to `target`, one FADE_STEP per sleep, clamped to the PWM
 * bounds. Signed maths on purpose -- stepping the uint8_t directly underflows. */
uint8_t bl_fade(uint8_t source, uint8_t target) {
    ARG_UNUSED(source); /* fade always starts from the live current_brightness */
    int level = current_brightness;
    int step = (target > level) ? FADE_STEP : -FADE_STEP;

    while (level != (int)target) {
        level += step;
        if (level > PWM_MAX) { level = PWM_MAX; }
        if (level < PWM_MIN) { level = PWM_MIN; }

        if (led_set_brightness(pwm_leds_dev, DISP_BL, (uint8_t)level)) {
            LOG_ERR("Failed to set brightness");
        }
        current_brightness = (uint8_t)level;

        if (level == PWM_MAX || level == PWM_MIN) {
            break; /* clamped -- can't get closer to target than this */
        }
        k_msleep(step > 0 ? FADE_SLEEP_BRIGHTEN_MS : FADE_SLEEP_DARKEN_MS);
    }

    return 0;
}

extern void als_thread(void *d0, void *d1, void *d2) {
    ARG_UNUSED(d0);
    ARG_UNUSED(d1);
    ARG_UNUSED(d2);

    const struct device *dev;
    struct sensor_value intensity;
    uint8_t mapped_brightness;

    dev = DEVICE_DT_GET_ONE(avago_apds9960);
    if (!device_is_ready(dev)) {
        printk("sensor: device not ready.\n");
    }

    // led_set_brightness(pwm_leds_dev, DISP_BL, 100);

    while (1) {

        k_msleep(NORMAL_SAMPLE_SLEEP_MS);


        if (sensor_sample_fetch(dev)) {
            LOG_ERR("sensor_sample fetch failed\n");
        }

        if (sensor_channel_get(dev, SENSOR_CHAN_LIGHT, &intensity)) {
            LOG_ERR("Cannot read ALS data.\n");
        }

        // LOG_INF("ambient light intensity %d", intensity.val1);

        mapped_brightness = map_light_to_pwm(intensity.val1);
        // LOG_INF("NORMAL: mapped PWM duty cycle %d\n", mapped_brightness);

        if (abs(mapped_brightness - current_brightness) > FADE_THRESHOLD) {
            uint8_t integrator = 0;

            for (int i = 0; i < BURST_SAMPLE_TIMEOUT; i++) {
                k_msleep(BURST_SAMPLE_SLEEP_MS);

                if (sensor_sample_fetch(dev)) {
                    LOG_ERR("sensor_sample fetch failed\n");
                }
                if (sensor_channel_get(dev, SENSOR_CHAN_LIGHT, &intensity)) {
                    LOG_ERR("Cannot read ALS data.\n");
                }

                mapped_brightness = map_light_to_pwm(intensity.val1);
                // LOG_INF("BURST: mapped PWM duty cycle %d\n", mapped_brightness);

                if (abs(mapped_brightness - current_brightness) > FADE_THRESHOLD) {
                    integrator++;
                    // printk("integrator at: %d", integrator);
                    if (integrator >= BURST_SAMPLE_CONSECUTIVE) {
                        bl_fade(current_brightness, mapped_brightness);
                        current_brightness = mapped_brightness;
                        // LOG_INF("SETTING NEW BRIGHTNESS: %d", mapped_brightness);
                        break;
                    }
                }
            }
        }
        // led_set_brightness(pwm_leds_dev, DISP_BL, map_light_to_pwm(intensity.val1));
    }
}

K_THREAD_DEFINE(als_tid, 1024, als_thread, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0,
                0);

#else

static int init_fixed_brightness(void) {
    led_set_brightness(pwm_leds_dev, DISP_BL, CONFIG_PROSPECTOR_FIXED_BRIGHTNESS);

    return 0;
}

SYS_INIT(init_fixed_brightness, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif