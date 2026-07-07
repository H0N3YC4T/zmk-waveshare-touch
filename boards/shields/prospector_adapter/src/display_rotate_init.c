#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

/* Boot orientation. MUST be DISPLAY_ORIENTATION_ROTATED_270 -- touch_rotation.c's
 * runtime rotation state (ui_rot, default 0 = "calibrated landscape") assumes the
 * panel boots into this exact orientation; rot_to_panel[0] in that file is the
 * same value. This used to be a Kconfig choice (PROSPECTOR_ROTATE_DISPLAY_180),
 * but that let the boot orientation and ui_rot's assumption drift out of sync --
 * flipping it would silently misalign every tap and the trackpad from boot,
 * with no obvious symptom pointing at the cause. The 4-way runtime rotation
 * (settings screen) supersedes the old static choice anyway, so it's removed;
 * this is now the one value the rest of the touch UI can rely on. */
int disp_set_orientation(void)
{
	const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display))
	{
		return -EIO;
	}

	return display_set_orientation(display, DISPLAY_ORIENTATION_ROTATED_270);
}

SYS_INIT(disp_set_orientation, APPLICATION, 60);