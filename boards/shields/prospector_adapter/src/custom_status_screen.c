#include <lvgl.h>

#if defined(CONFIG_PROSPECTOR_STATUS_SCREEN_OPERATOR)
#include "layouts/operator/status_screen.c"
#else
#error "No status screen layout selected (this module carries OPERATOR only)"
#endif
