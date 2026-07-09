/* Key/behaviour sending for the touch UI -- the thread-safety boundary.
 *
 * Sends are marshalled onto the system workqueue. ZMK's HID and endpoint code has
 * no cross-thread locking: zmk_behavior_invoke_binding() writes the shared keyboard
 * report on whichever thread calls it, so invoking from the LVGL display thread
 * races ZMK's own event processing and corrupts the report. Note that
 * zmk_behavior_queue_add() is not a thread hop by itself -- with wait=0 it runs the
 * behavior synchronously on the caller (see zmk's app/src/behavior_queue.c). The hop
 * has to come from our own k_work_submit(); queue_add is then called inside the
 * handler, where it also serialises with any in-flight delayed macro. */

#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keymap.h> /* ZMK_KEYMAP_EXTRACT_BINDING (PAD bindings) */

#include "../touch_ui.h"

uint8_t pending_mods; /* one-shot, applied to the next key sent */

struct pending_key {
    const char *dev;
    uint32_t param1;
    uint32_t param2;
};
#define KEY_RING_SZ 8
static struct pending_key key_ring[KEY_RING_SZ];
static atomic_t key_ring_head = ATOMIC_INIT(0);
static atomic_t key_ring_tail = ATOMIC_INIT(0);

static void key_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    while (atomic_get(&key_ring_tail) != atomic_get(&key_ring_head)) {
        struct pending_key pk = key_ring[atomic_get(&key_ring_tail) % KEY_RING_SZ];
        struct zmk_behavior_binding b = {.behavior_dev = pk.dev, .param1 = pk.param1,
                                         .param2 = pk.param2};
        struct zmk_behavior_binding_event ev = {.layer = 0, .position = 0x7000,
                                                .timestamp = k_uptime_get()};
        /* Runs on the system workqueue thread (this handler's context), so queue_add's
         * synchronous fast-path is safe here -- it is NOT the display thread. */
        zmk_behavior_queue_add(&ev, b, true, 0);
        zmk_behavior_queue_add(&ev, b, false, 0);
        atomic_inc(&key_ring_tail);
    }
}
static K_WORK_DEFINE(key_work, key_work_handler);

static void queue_key(const char *dev, uint32_t param1, uint32_t param2) {
    int h = atomic_get(&key_ring_head);
    key_ring[h % KEY_RING_SZ] =
        (struct pending_key){.dev = dev, .param1 = param1, .param2 = param2};
    atomic_set(&key_ring_head, h + 1); /* release: publishes the ring slot */
    k_work_submit(&key_work);
}

/* Send a regular key press via ZMK's &kp behaviour, applying any one-shot mod. */
void send_key(uint32_t keycode) {
    queue_key("key_press", keycode | ((uint32_t)pending_mods << 24), 0);
    pending_mods = 0;
}

void fire_macro(const char *dev) {
    queue_key(dev, 0, 0);
}

/* PAD buttons: keymap-syntax bindings (&kp, &bt, macros, ...) from the consuming
 * keyboard's zmk,prospector-touch-pad node. One-shot mods do NOT apply here (they
 * ride inside send_key's param encoding); bake mods into the binding instead. */
#if DT_HAS_COMPAT_STATUS_OKAY(zmk_prospector_touch_pad)
#define PAD_NODE DT_INST(0, zmk_prospector_touch_pad)
static const struct zmk_behavior_binding pad_bindings[] = {
    LISTIFY(DT_PROP_LEN(PAD_NODE, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), PAD_NODE)};
int touch_pad_count(void) { return ARRAY_SIZE(pad_bindings); }
void fire_pad(int idx) {
    if (idx >= 0 && idx < (int)ARRAY_SIZE(pad_bindings)) {
        const struct zmk_behavior_binding *b = &pad_bindings[idx];
        queue_key(b->behavior_dev, b->param1, b->param2);
    }
}
#else
int touch_pad_count(void) { return 0; }
void fire_pad(int idx) { ARG_UNUSED(idx); }
#endif
