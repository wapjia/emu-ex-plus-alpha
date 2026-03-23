#pragma once

#include <stdbool.h>

static int ui_set_aspect_mode(int mode, void *canvas) { return 0; }
static int ui_set_aspect_ratio(double aspect, void *canvas) { return 0; }
static int ui_set_glfilter(int val, void *canvas) { return 0; }
static int ui_set_flipx(int val, void *canvas) { return 0; }
static int ui_set_flipy(int val, void *canvas) { return 0; }
static int ui_set_rotate(int val, void *canvas) { return 0; }
static int ui_set_vsync(int val, void *canvas) { return 0; }

static int ui_pause_active() { return 0; }
static void ui_pause_enable() {}
static void ui_pause_disable() {}
static bool ui_pause_loop_iteration(void) { return false; }
