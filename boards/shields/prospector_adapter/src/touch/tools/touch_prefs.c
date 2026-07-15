/* Persist brightness / trackpad sensitivity / rotation ("prospector_ui/prefs"). */

#include <zephyr/settings/settings.h>

#include "../touch_ui.h"

#if IS_ENABLED(CONFIG_SETTINGS)

struct ui_prefs {
    uint8_t bright;
    int8_t sens;
    uint8_t rot;
};

static struct ui_prefs loaded_prefs;
static bool prefs_loaded;

static void prefs_save_cb(struct k_work *work) {
    struct ui_prefs p = {
        .bright = prospector_brightness_get(),
        .sens = (int8_t)prospector_touchpad_sens_get(),
        .rot = ui_rot,
    };
    settings_save_one("prospector_ui/prefs", &p, sizeof(p));
}
static K_WORK_DELAYABLE_DEFINE(prefs_save_work, prefs_save_cb);

void touch_prefs_save(void) { k_work_schedule(&prefs_save_work, K_SECONDS(2)); }

/* called once from status-screen creation, after the touch UI is attached */
void touch_prefs_apply(void) {
    if (!prefs_loaded) {
        return;
    }
    prospector_brightness_step((int)loaded_prefs.bright - prospector_brightness_get());
    if (loaded_prefs.sens >= 0 && prospector_touchpad_sens_get() >= 0) {
        prospector_touchpad_sens_step(loaded_prefs.sens - prospector_touchpad_sens_get());
    }
    if ((loaded_prefs.rot & 3) != ui_rot) {
        ui_rot = loaded_prefs.rot & 3;
        settings_apply_rotation();
    }
}

static int prefs_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                              void *cb_arg) {
    if (settings_name_steq(name, "prefs", NULL) && len == sizeof(loaded_prefs)) {
        if (read_cb(cb_arg, &loaded_prefs, sizeof(loaded_prefs)) == sizeof(loaded_prefs)) {
            prefs_loaded = true;
        }
    }
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(prospector_ui, "prospector_ui", NULL, prefs_settings_set, NULL,
                               NULL);

#else /* !CONFIG_SETTINGS */

void touch_prefs_save(void) {}
void touch_prefs_apply(void) {}

#endif
