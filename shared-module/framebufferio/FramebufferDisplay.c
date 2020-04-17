/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Scott Shawcroft for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "shared-bindings/framebufferio/FramebufferDisplay.h"

#include "py/gc.h"
#include "py/runtime.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/time/__init__.h"
#include "shared-module/displayio/__init__.h"
#include "shared-module/displayio/display_core.h"
#include "supervisor/shared/display.h"
#include "supervisor/shared/tick.h"
#include "supervisor/usb.h"

#include <stdint.h>
#include <string.h>

#include "tick.h"

void common_hal_framebufferio_framebufferdisplay_construct(framebufferio_framebufferdisplay_obj_t* self,
        mp_obj_t framebuffer, uint16_t width, uint16_t height,
        uint16_t rotation, uint16_t color_depth,
        uint8_t bytes_per_cell,
        bool auto_refresh, uint16_t native_frames_per_second) {
    // Turn off auto-refresh as we init.
    self->auto_refresh = false;
    self->framebuffer = framebuffer;
    self->framebuffer_protocol = mp_proto_get_or_throw(MP_QSTR_protocol_framebuffer, framebuffer);

    uint16_t ram_width = 0x100;
    uint16_t ram_height = 0x100;

    displayio_display_core_construct(&self->core, NULL, width, height, ram_width, ram_height, 0, 0, rotation,
        color_depth, false, false, bytes_per_cell, false, false);

    self->first_manual_refresh = !auto_refresh;

    self->native_frames_per_second = native_frames_per_second;
    self->native_ms_per_frame = 1000 / native_frames_per_second;

    supervisor_start_terminal(width, height);

    // Set the group after initialization otherwise we may send pixels while we delay in
    // initialization.
    common_hal_framebufferio_framebufferdisplay_show(self, &circuitpython_splash);
    self->auto_refresh = auto_refresh;
}

bool common_hal_framebufferio_framebufferdisplay_show(framebufferio_framebufferdisplay_obj_t* self, displayio_group_t* root_group) {
    return displayio_display_core_show(&self->core, root_group);
}

uint16_t common_hal_framebufferio_framebufferdisplay_get_width(framebufferio_framebufferdisplay_obj_t* self){
    return displayio_display_core_get_width(&self->core);
}

uint16_t common_hal_framebufferio_framebufferdisplay_get_height(framebufferio_framebufferdisplay_obj_t* self){
    return displayio_display_core_get_height(&self->core);
}

bool common_hal_framebufferio_framebufferdisplay_get_auto_brightness(framebufferio_framebufferdisplay_obj_t* self) {
    if (self->framebuffer_protocol->get_auto_brightness) {
        return self->framebuffer_protocol->get_auto_brightness(self->framebuffer);
    }
    return true;
}

bool common_hal_framebufferio_framebufferdisplay_set_auto_brightness(framebufferio_framebufferdisplay_obj_t* self, bool auto_brightness) {
    if (self->framebuffer_protocol->set_auto_brightness) {
        return self->framebuffer_protocol->set_auto_brightness(self->framebuffer, auto_brightness);
    }
    return false;
}

mp_float_t common_hal_framebufferio_framebufferdisplay_get_brightness(framebufferio_framebufferdisplay_obj_t* self) {
    if (self->framebuffer_protocol->set_brightness) {
        return self->framebuffer_protocol->get_brightness(self->framebuffer);
    }
    return -1;
}

bool common_hal_framebufferio_framebufferdisplay_set_brightness(framebufferio_framebufferdisplay_obj_t* self, mp_float_t brightness) {
    bool ok = false;
    if (self->framebuffer_protocol->set_brightness) {
        self->framebuffer_protocol->set_brightness(self->framebuffer, brightness);
        ok = true;
    }
    return ok;
}

mp_obj_t common_hal_framebufferio_framebufferdisplay_get_framebuffer(framebufferio_framebufferdisplay_obj_t* self) {
    return self->framebuffer;
}

STATIC const displayio_area_t* _get_refresh_areas(framebufferio_framebufferdisplay_obj_t *self) {
    if (self->core.full_refresh) {
        self->core.area.next = NULL;
        return &self->core.area;
    } else if (self->core.current_group != NULL) {
        return displayio_group_get_refresh_areas(self->core.current_group, NULL);
    }
    return NULL;
}

STATIC bool _refresh_area(framebufferio_framebufferdisplay_obj_t* self, const displayio_area_t* area) {
    uint16_t buffer_size = 128; // In uint32_ts

    displayio_area_t clipped;
    // Clip the area to the display by overlapping the areas. If there is no overlap then we're done.
    if (!displayio_display_core_clip_area(&self->core, area, &clipped)) {
        return true;
    }
    uint16_t subrectangles = 1;
    uint16_t rows_per_buffer = displayio_area_height(&clipped);
    uint8_t pixels_per_word = (sizeof(uint32_t) * 8) / self->core.colorspace.depth;
    uint16_t pixels_per_buffer = displayio_area_size(&clipped);
    if (displayio_area_size(&clipped) > buffer_size * pixels_per_word) {
        rows_per_buffer = buffer_size * pixels_per_word / displayio_area_width(&clipped);
        if (rows_per_buffer == 0) {
            rows_per_buffer = 1;
        }
        // If pixels are packed by column then ensure rows_per_buffer is on a byte boundary.
        if (self->core.colorspace.depth < 8 && !self->core.colorspace.pixels_in_byte_share_row) {
            uint8_t pixels_per_byte = 8 / self->core.colorspace.depth;
            if (rows_per_buffer % pixels_per_byte != 0) {
                rows_per_buffer -= rows_per_buffer % pixels_per_byte;
            }
        }
        subrectangles = displayio_area_height(&clipped) / rows_per_buffer;
        if (displayio_area_height(&clipped) % rows_per_buffer != 0) {
            subrectangles++;
        }
        pixels_per_buffer = rows_per_buffer * displayio_area_width(&clipped);
        buffer_size = pixels_per_buffer / pixels_per_word;
        if (pixels_per_buffer % pixels_per_word) {
            buffer_size += 1;
        }
    }

    // Allocated and shared as a uint32_t array so the compiler knows the
    // alignment everywhere.
    uint32_t buffer[buffer_size];
    uint32_t mask_length = (pixels_per_buffer / 32) + 1;
    uint32_t mask[mask_length];
    uint16_t remaining_rows = displayio_area_height(&clipped);

    for (uint16_t j = 0; j < subrectangles; j++) {
        displayio_area_t subrectangle = {
            .x1 = clipped.x1,
            .y1 = clipped.y1 + rows_per_buffer * j,
            .x2 = clipped.x2,
            .y2 = clipped.y1 + rows_per_buffer * (j + 1)
        };
        if (remaining_rows < rows_per_buffer) {
            subrectangle.y2 = subrectangle.y1 + remaining_rows;
        }
        remaining_rows -= rows_per_buffer;

        memset(mask, 0, mask_length * sizeof(mask[0]));
        memset(buffer, 0, buffer_size * sizeof(buffer[0]));

        displayio_display_core_fill_area(&self->core, &subrectangle, mask, buffer);

        // COULDDO: this arithmetic only supports multiple-of-8 bpp
        uint8_t *dest = self->bufinfo.buf + (subrectangle.y1 * self->core.width + subrectangle.x1) * (self->core.colorspace.depth / 8);
        uint8_t *src = (uint8_t*)buffer;
        size_t rowsize = (subrectangle.x2 - subrectangle.x1) * (self->core.colorspace.depth / 8);
        size_t rowstride = self->core.width * (self->core.colorspace.depth/8);
        for (uint16_t i = subrectangle.y1; i < subrectangle.y2; i++) {
            memcpy(dest, src, rowsize);
            dest += rowstride;
            src += rowsize;
        }

        // TODO(tannewt): Make refresh displays faster so we don't starve other
        // background tasks.
        usb_background();
    }
    return true;
}

STATIC void _refresh_display(framebufferio_framebufferdisplay_obj_t* self) {
    displayio_display_core_start_refresh(&self->core);
    self->framebuffer_protocol->get_bufinfo(self->framebuffer, &self->bufinfo);
    const displayio_area_t* current_area = _get_refresh_areas(self);
    while (current_area != NULL) {
        _refresh_area(self, current_area);
        current_area = current_area->next;
    }
    displayio_display_core_finish_refresh(&self->core);
    self->framebuffer_protocol->swapbuffers(self->framebuffer);
}

void common_hal_framebufferio_framebufferdisplay_set_rotation(framebufferio_framebufferdisplay_obj_t* self, int rotation){
    bool transposed = (self->core.rotation == 90 || self->core.rotation == 270);
    bool will_transposed = (rotation == 90 || rotation == 270);
    if(transposed != will_transposed) {
        int tmp = self->core.width;
        self->core.width = self->core.height;
        self->core.height = tmp;
    }
    displayio_display_core_set_rotation(&self->core, rotation);
    supervisor_stop_terminal();
    supervisor_start_terminal(self->core.width, self->core.height);
    if (self->core.current_group != NULL) {
        displayio_group_update_transform(self->core.current_group, &self->core.transform);
    }
}

uint16_t common_hal_framebufferio_framebufferdisplay_get_rotation(framebufferio_framebufferdisplay_obj_t* self){
    return self->core.rotation;
}


bool common_hal_framebufferio_framebufferdisplay_refresh(framebufferio_framebufferdisplay_obj_t* self, uint32_t target_ms_per_frame, uint32_t maximum_ms_per_real_frame) {
    if (!self->auto_refresh && !self->first_manual_refresh) {
        uint64_t current_time = supervisor_ticks_ms64();
        uint32_t current_ms_since_real_refresh = current_time - self->core.last_refresh;
        // Test to see if the real frame time is below our minimum.
        if (current_ms_since_real_refresh > maximum_ms_per_real_frame) {
            mp_raise_RuntimeError(translate("Below minimum frame rate"));
        }
        uint32_t current_ms_since_last_call = current_time - self->last_refresh_call;
        self->last_refresh_call = current_time;
        // Skip the actual refresh to help catch up.
        if (current_ms_since_last_call > target_ms_per_frame) {
            return false;
        }
        uint32_t remaining_time = target_ms_per_frame - (current_ms_since_real_refresh % target_ms_per_frame);
        // We're ahead of the game so wait until we align with the frame rate.
        while (supervisor_ticks_ms64() - self->last_refresh_call < remaining_time) {
            RUN_BACKGROUND_TASKS;
        }
    }
    self->first_manual_refresh = false;
    _refresh_display(self);
    return true;
}

bool common_hal_framebufferio_framebufferdisplay_get_auto_refresh(framebufferio_framebufferdisplay_obj_t* self) {
    return self->auto_refresh;
}

void common_hal_framebufferio_framebufferdisplay_set_auto_refresh(framebufferio_framebufferdisplay_obj_t* self,
                                                   bool auto_refresh) {
    self->first_manual_refresh = !auto_refresh;
    self->auto_refresh = auto_refresh;
}

STATIC void _update_backlight(framebufferio_framebufferdisplay_obj_t* self) {
    // TODO(tannewt): Fade the backlight based on it's existing value and a target value. The target
    // should account for ambient light when possible.
}

void framebufferio_framebufferdisplay_background(framebufferio_framebufferdisplay_obj_t* self) {
    _update_backlight(self);

    if (self->auto_refresh && (supervisor_ticks_ms64() - self->core.last_refresh) > self->native_ms_per_frame) {
        _refresh_display(self);
    }
}

void release_framebufferdisplay(framebufferio_framebufferdisplay_obj_t* self) {
    release_display_core(&self->core);
    self->framebuffer_protocol->deinit(self->framebuffer);
}

void reset_framebufferdisplay(framebufferio_framebufferdisplay_obj_t* self) {
    self->auto_refresh = true;
    common_hal_framebufferio_framebufferdisplay_show(self, NULL);
}

void framebufferio_framebufferdisplay_collect_ptrs(framebufferio_framebufferdisplay_obj_t* self) {
    gc_collect_ptr(self->framebuffer);
    displayio_display_core_collect_ptrs(&self->core);
}
