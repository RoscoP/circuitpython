/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Mark Komus
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

// This file contains all of the Python API definitions for the
// busio.SPI class.

#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/busdevice/SPIDevice.h"
#include "shared-bindings/util.h"
#include "shared-module/busdevice/SPIDevice.h"
#include "common-hal/digitalio/DigitalInOut.h"
#include "shared-bindings/digitalio/DigitalInOut.h"


#include "lib/utils/buffer_helper.h"
#include "lib/utils/context_manager_helpers.h"
#include "py/runtime.h"
#include "supervisor/shared/translate.h"


//| class SPIDevice:
//|     """
//|     Represents a single SPI device and manages locking the bus and the device
//|     address.
//|     :param ~busio.SPI spi: The SPI bus the device is on
//|     :param int device_address: The 7 bit device address
//|     :param bool probe: Probe for the device upon object creation, default is true
//|     .. note:: This class is **NOT** built into CircuitPython. See
//|       :ref:`here for install instructions <bus_device_installation>`.
//|     Example:
//|     .. code-block:: python
//|         import busio
//|         from board import *
//|         from adafruit_bus_device.spi_device import SPIDevice
//|         with busio.SPI(SCL, SDA) as spi:
//|             device = SPIDevice(spi, 0x70)
//|             bytes_read = bytearray(4)
//|             with device:
//|                 device.readinto(bytes_read)
//|             # A second transaction
//|             with device:
//|                 device.write(bytes_read)"""
//|     ...
//|
STATIC mp_obj_t busdevice_spidevice_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    busdevice_spidevice_obj_t *self = m_new_obj(busdevice_spidevice_obj_t);
    self->base.type = &busdevice_spidevice_type;
    enum { ARG_spi, ARG_chip_select, ARG_baudrate, ARG_polarity, ARG_phase, ARG_extra_clocks };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_spi, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_chip_select, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 100000} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_extra_clocks, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    busio_spi_obj_t* spi = args[ARG_spi].u_obj;

    common_hal_busdevice_spidevice_construct(MP_OBJ_TO_PTR(self), spi, args[ARG_chip_select].u_obj, args[ARG_baudrate].u_int, args[ARG_polarity].u_int,
        args[ARG_phase].u_int, args[ARG_extra_clocks].u_int);

    if (args[ARG_chip_select].u_obj != MP_OBJ_NULL) {
        digitalinout_result_t result = common_hal_digitalio_digitalinout_switch_to_output(MP_OBJ_TO_PTR(args[ARG_chip_select].u_obj),
            true, DRIVE_MODE_PUSH_PULL);
        if (result == DIGITALINOUT_INPUT_ONLY) {
            mp_raise_NotImplementedError(translate("Pin is input only"));
        }
    }

    return (mp_obj_t)self;
}

STATIC mp_obj_t busdevice_spidevice_obj___enter__(mp_obj_t self_in) {
    busdevice_spidevice_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_busdevice_spidevice_enter(self);
    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(busdevice_spidevice___enter___obj, busdevice_spidevice_obj___enter__);

STATIC mp_obj_t busdevice_spidevice_obj___exit__(size_t n_args, const mp_obj_t *args) {
    common_hal_busdevice_spidevice_exit(MP_OBJ_TO_PTR(args[0]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(busdevice_spidevice___exit___obj, 4, 4, busdevice_spidevice_obj___exit__);

STATIC const mp_rom_map_elem_t busdevice_spidevice_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&busdevice_spidevice___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&busdevice_spidevice___exit___obj) },
};

STATIC MP_DEFINE_CONST_DICT(busdevice_spidevice_locals_dict, busdevice_spidevice_locals_dict_table);

const mp_obj_type_t busdevice_spidevice_type = {
   { &mp_type_type },
   .name = MP_QSTR_SPIDevice,
   .make_new = busdevice_spidevice_make_new,
   .locals_dict = (mp_obj_dict_t*)&busdevice_spidevice_locals_dict,
};
