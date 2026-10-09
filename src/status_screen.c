/*
 * Custom ZMK status screen for a physically sideways-mounted OLED.
 *
 * ZMK's stock status screen draws widgets straight onto the display's
 * native (landscape) orientation. Our panel is mounted rotated 90
 * degrees on the PCB, so that output reads sideways to the user.
 *
 * The SSD1306 driver has no hardware rotation, and LVGL's whole-display
 * software rotation is known broken for monochrome panels (see
 * zmkfirmware/zmk#1749). The workaround used here: lay the same widgets
 * out in an off-screen portrait container sized to the panel's long
 * axis, periodically snapshot that container with lv_snapshot_take(),
 * and display the snapshot as a rotated image on the real screen.
 */

#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/output_status.h>
#include <zmk/display/widgets/peripheral_status.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/status_screen.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
static struct zmk_widget_battery_status battery_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
static struct zmk_widget_output_status output_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
static struct zmk_widget_peripheral_status peripheral_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
static struct zmk_widget_layer_status layer_status_widget;
#endif

static lv_obj_t *content;
static lv_obj_t *rotated_img;
static lv_draw_buf_t *snapshot_buf;

static void refresh_snapshot(lv_timer_t *timer) {
    ARG_UNUSED(timer);

    lv_draw_buf_t *old = snapshot_buf;

    snapshot_buf = lv_snapshot_take(content, LV_COLOR_FORMAT_I1);
    if (snapshot_buf == NULL) {
        LOG_WRN("lily58-extras: snapshot failed, keeping previous frame");
        snapshot_buf = old;
        return;
    }

    lv_image_set_src(rotated_img, snapshot_buf);

    if (old != NULL) {
        lv_draw_buf_destroy(old);
    }
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);

    content = lv_obj_create(screen);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, CONFIG_ZMK_LILY58_EXTRAS_CONTENT_WIDTH,
                     CONFIG_ZMK_LILY58_EXTRAS_CONTENT_HEIGHT);
    lv_obj_set_pos(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    /* Stays off the real screen tree's visible render, but LVGL still
     * needs it laid out and drawn normally so it can be snapshotted. */
    lv_obj_add_flag(content, LV_OBJ_FLAG_HIDDEN);

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
    zmk_widget_battery_status_init(&battery_status_widget, content);
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
    zmk_widget_output_status_init(&output_status_widget, content);
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
    zmk_widget_peripheral_status_init(&peripheral_status_widget, content);
#endif

#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
    zmk_widget_layer_status_init(&layer_status_widget, content);
#endif

    rotated_img = lv_image_create(screen);
    lv_image_set_rotation(rotated_img, CONFIG_ZMK_LILY58_EXTRAS_DISPLAY_ROTATION);
    lv_obj_center(rotated_img);

    /* LV_OBJ_FLAG_HIDDEN skips an object's own render but LVGL still
     * lays out and draws a hidden widget's *children* when something
     * else (lv_snapshot_take) asks for it, so battery/layer/etc. still
     * render correctly into the snapshot despite `content` itself being
     * hidden from the normal screen render pass. */
    refresh_snapshot(NULL);
    lv_timer_create(refresh_snapshot, CONFIG_ZMK_LILY58_EXTRAS_REFRESH_MS, NULL);

    return screen;
}
