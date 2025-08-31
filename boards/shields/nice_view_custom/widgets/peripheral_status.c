/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>

#include "peripheral_status.h"

LV_IMG_DECLARE(frame_000);
LV_IMG_DECLARE(frame_001);
LV_IMG_DECLARE(frame_002);
LV_IMG_DECLARE(frame_003);
LV_IMG_DECLARE(frame_004);
LV_IMG_DECLARE(frame_005);
LV_IMG_DECLARE(frame_006);
LV_IMG_DECLARE(frame_007);
LV_IMG_DECLARE(frame_008);
LV_IMG_DECLARE(frame_009);
LV_IMG_DECLARE(frame_010);
LV_IMG_DECLARE(frame_011);
LV_IMG_DECLARE(frame_012);
LV_IMG_DECLARE(frame_013);
LV_IMG_DECLARE(frame_014);
LV_IMG_DECLARE(frame_015);
LV_IMG_DECLARE(frame_016);
LV_IMG_DECLARE(frame_017);
LV_IMG_DECLARE(frame_018);
LV_IMG_DECLARE(frame_019);
LV_IMG_DECLARE(frame_020);
LV_IMG_DECLARE(frame_021);
LV_IMG_DECLARE(frame_022);
LV_IMG_DECLARE(frame_023);
LV_IMG_DECLARE(frame_024);
LV_IMG_DECLARE(frame_025);
LV_IMG_DECLARE(frame_026);
LV_IMG_DECLARE(frame_027);
LV_IMG_DECLARE(frame_028);
LV_IMG_DECLARE(frame_029);
LV_IMG_DECLARE(frame_030);
LV_IMG_DECLARE(frame_031);
LV_IMG_DECLARE(frame_032);
LV_IMG_DECLARE(frame_033);
LV_IMG_DECLARE(frame_034);
LV_IMG_DECLARE(frame_035);
LV_IMG_DECLARE(frame_036);
LV_IMG_DECLARE(frame_037);
LV_IMG_DECLARE(frame_038);
LV_IMG_DECLARE(frame_039);
LV_IMG_DECLARE(frame_040);
LV_IMG_DECLARE(frame_041);
LV_IMG_DECLARE(frame_042);
LV_IMG_DECLARE(frame_043);
LV_IMG_DECLARE(frame_044);
LV_IMG_DECLARE(frame_045);
LV_IMG_DECLARE(frame_046);
LV_IMG_DECLARE(frame_047);
LV_IMG_DECLARE(frame_048);
LV_IMG_DECLARE(frame_049);
LV_IMG_DECLARE(frame_050);
LV_IMG_DECLARE(frame_051);
LV_IMG_DECLARE(frame_052);
LV_IMG_DECLARE(frame_053);
LV_IMG_DECLARE(frame_054);
LV_IMG_DECLARE(frame_055);
LV_IMG_DECLARE(frame_056);
LV_IMG_DECLARE(frame_057);

const lv_img_dsc_t *anim_imgs[] = {
    &frame_000,
    &frame_001,
    &frame_002,
    &frame_003,
    &frame_004,
    &frame_005,
    &frame_006,
    &frame_007,
    &frame_008,
    &frame_009,
    &frame_010,
    &frame_011,
    &frame_012,
    &frame_013,
    &frame_014,
    &frame_015,
    &frame_016,
    &frame_017,
    &frame_018,
    &frame_019,
    &frame_020,
    &frame_021,
    &frame_022,
    &frame_023,
    &frame_024,
    &frame_025,
    &frame_026,
    &frame_027,
    &frame_028,
    &frame_029,
    &frame_030,
    &frame_031,
    &frame_032,
    &frame_033,
    &frame_034,
    &frame_035,
    &frame_036,
    &frame_037,
    &frame_038,
    &frame_039,
    &frame_040,
    &frame_041,
    &frame_042,
    &frame_043,
    &frame_044,
    &frame_045,
    &frame_046,
    &frame_047,
    &frame_048,
    &frame_049,
    &frame_050,
    &frame_051,
    &frame_052,
    &frame_053,
    &frame_054,
    &frame_055,
    &frame_056,
    &frame_057,
};

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct peripheral_status_state {
    bool connected;
};

static void draw_top(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_RIGHT);
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);

    // Fill background
    lv_canvas_draw_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, &rect_black_dsc);

    // Draw battery
    draw_battery(canvas, state);

    // Draw output status
    lv_canvas_draw_text(canvas, 0, 0, CANVAS_SIZE, &label_dsc,
                        state->connected ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE);

    // Rotate canvas
    rotate_canvas(canvas, cbuf);
}

static void set_battery_status(struct zmk_widget_status *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    widget->state.battery = state.level;

    draw_top(widget->obj, widget->cbuf, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    return (struct battery_status_state) {
        .level = zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

static struct peripheral_status_state get_state(const zmk_event_t *_eh) {
    return (struct peripheral_status_state){.connected = zmk_split_bt_peripheral_is_connected()};
}

static void set_connection_status(struct zmk_widget_status *widget,
                                  struct peripheral_status_state state) {
    widget->state.connected = state.connected;

    draw_top(widget->obj, widget->cbuf, &widget->state);
}

static void output_status_update_cb(struct peripheral_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_connection_status(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_status, struct peripheral_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_peripheral_status, zmk_split_peripheral_status_changed);

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 160, 68);
    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);

    //lv_obj_t *art = lv_img_create(widget->obj);
    //bool random = sys_rand32_get() & 1;
    //lv_img_set_src(art, random ? &balloon : &mountain);
    //lv_img_set_src(art, &corro01);

    lv_obj_t * art = lv_animimg_create(widget->obj);            //<--
    lv_obj_center(art);                                         //<--
    lv_animimg_set_src(art, (const void **) anim_imgs, 25);     //<--
    lv_animimg_set_duration(art, CONFIG_CUSTOM_ANIMATION_SPEED);//<--
    lv_animimg_set_repeat_count(art, LV_ANIM_REPEAT_INFINITE);  //<--
    lv_animimg_start(art);                                      //<--
    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 0, 0);
    
    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_peripheral_status_init();

    return 0;
}


lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
