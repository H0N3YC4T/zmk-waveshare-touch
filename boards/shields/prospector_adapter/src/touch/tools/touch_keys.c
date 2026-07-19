
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keymap.h> /* ZMK_KEYMAP_EXTRACT_BINDING (PAD bindings) */

#include "../touch_ui.h"

uint8_t pending_mods; /* one-shot, applied to the next key sent */
enum theme_role pending_mod_role = THEME_SECONDARY;

struct pending_key
{
  const char *dev;
  uint32_t param1;
  uint32_t param2;
};
#define KEY_RING_SZ 8
static struct pending_key key_ring[KEY_RING_SZ];
static atomic_t key_ring_head = ATOMIC_INIT(0);
static atomic_t key_ring_tail = ATOMIC_INIT(0);

/* all sends run here (workqueue); queue_add wait=0 is NOT a thread hop */
static void key_work_handler(struct k_work *work)
{
  ARG_UNUSED(work);
  while (atomic_get(&key_ring_tail) != atomic_get(&key_ring_head))
  {
    struct pending_key pk = key_ring[atomic_get(&key_ring_tail) % KEY_RING_SZ];
    struct zmk_behavior_binding b = {.behavior_dev = pk.dev, .param1 = pk.param1, .param2 = pk.param2};
    struct zmk_behavior_binding_event ev = {.layer = 0, .position = 0x7000, .timestamp = k_uptime_get()};
    zmk_behavior_queue_add(&ev, b, true, 0);
    zmk_behavior_queue_add(&ev, b, false, 0);
    atomic_inc(&key_ring_tail);
  }
}

static K_WORK_DEFINE(key_work, key_work_handler);

static void queue_key(const char *dev, uint32_t param1, uint32_t param2)
{
  int h = atomic_get(&key_ring_head);
  key_ring[h % KEY_RING_SZ] =
      (struct pending_key){.dev = dev, .param1 = param1, .param2 = param2};
  atomic_set(&key_ring_head, h + 1); /* release: publishes the ring slot */
  k_work_submit(&key_work);
}

/* Send a regular key press via ZMK's &kp behaviour, applying any one-shot mod. */
void send_key(uint32_t keycode)
{
  queue_key("key_press", keycode | ((uint32_t)pending_mods << 24), 0);
  pending_mods = 0;
}

void fire_macro(const char *dev)
{
  queue_key(dev, 0, 0);
}

#if DT_HAS_COMPAT_STATUS_OKAY(zmk_prospector_touch_pad)
#define PAD_NODE DT_INST(0, zmk_prospector_touch_pad)
static const struct zmk_behavior_binding pad_bindings[] = {
    LISTIFY(DT_PROP_LEN(PAD_NODE, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), PAD_NODE)};

int touch_pad_count(void) { return ARRAY_SIZE(pad_bindings); }

void fire_pad(int idx)
{
  if (idx >= 0 && idx < (int)ARRAY_SIZE(pad_bindings))
  {
    const struct zmk_behavior_binding *b = &pad_bindings[idx];
    queue_key(b->behavior_dev, b->param1, b->param2);
  }
}

#else
int touch_pad_count(void) { return 0; }
void fire_pad(int idx) { ARG_UNUSED(idx); }
#endif
