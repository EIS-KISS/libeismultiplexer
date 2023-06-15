/**
* Copyright (C) 2023 Carl Klemm
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 3 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
*/

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
	
#define CHANNEL_A 1
#define CHANNEL_B (1 << 1)
#define CHANNEL_C (1 << 2)
#define CHANNEL_D (1 << 3)
	
struct uvosled {
	struct usbshm* priv;
};

// returns 0 on sucess and < 0 on failure
int uvosled_connect(struct uvosled* led);

// power on cameras
// returns 0 on sucess and < 0 on failure
int uvosled_poweron(struct uvosled* led);

// power off cameras
// returns 0 on sucess and < 0 on failure
int uvosled_poweroff(struct uvosled* led);

// channels is a mask of bits, you can set it like this: CHANNEL_A | CHANNEL_C to activate channels A and C
// current is in percent of the values selected by the linear regulator resistors
// returns 0 on sucess and < 0 on failure
int uvosled_set_current(struct uvosled* led, uint8_t channels, float current);

// causes the cameras to take an image
// returns 0 on sucess and < 0 on failure
int uvosled_trigger(struct uvosled* led);

// leds are lit for time seconds and the camera is activated cameraOffset seconds after they are lit
// real time guarenteed by microcontroller
// returns 0 on sucess and < 0 on failure
int uvosled_capture(struct uvosled* led, int channels, float current, double time, double cameraOffset);

void uvosled_disconnect(struct uvosled* led);

#ifdef __cplusplus
}
#endif
