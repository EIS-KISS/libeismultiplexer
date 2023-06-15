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

#define _POSIX_C_SOURCE 199309L
#include "uvosled.h"
#include "usbshm.h"
#include <stdlib.h>
#include <time.h>

void usleep(uint64_t microseconds)
{
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

int uvosled_connect(struct uvosled* led)
{
	int ret;
	led->priv = malloc(sizeof(*led->priv));
	if(!led->priv)
		return -1;
	
	ret = usbshm_init(led->priv, NULL);
	if(ret)
		return -2;
	
	ret = usbshm_open(led->priv, 0xfe17, 0x06dc , NULL);
	if(ret)
		return -3;
	return 0;
}

int uvosled_poweron(struct uvosled* led)
{
	int ret;
	while((ret = usbshm_writeControlTransfer(led->priv, 0, NULL, 0, 0, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

int uvosled_poweroff(struct uvosled* led)
{
	int ret;
	while((ret = usbshm_writeControlTransfer(led->priv, 1, NULL, 0, 0, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

int uvosled_set_current(struct uvosled* led, uint8_t channels, float current)
{
	if(current < 0) 
		return -1;
	else if(current > 1)
		return -2;
	uint8_t currentU = current * 255;
	// TODO: implment endianess
	uint16_t wValue;
	// we compile with fno-strict-aliasing
	uint8_t* wValChar = (uint8_t*)&wValue;
	wValChar[0] = channels;
	wValChar[1] = currentU;
	int ret;
	while((ret = usbshm_writeControlTransfer(led->priv, 2, NULL, 0, wValue, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

int uvosled_trigger(struct uvosled* led)
{
	int ret;
	while((ret = usbshm_writeControlTransfer(led->priv, 1, NULL, 0, 0, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

int uvosled_capture(struct uvosled* led, int channels, float current, double time, double cameraOffset)
{
	if(current < 0 || current > 1 || time > 1 || cameraOffset < -1 || cameraOffset > 1) 
		return USBSHM_ERROR_PARAM;
	
	uint8_t currentU = current * 255;
	uint16_t timeU = time * 1000;
	int16_t cameraOffsetU = cameraOffset * 1000;
	int ret;
	
	while((ret = usbshm_writeControlTransfer(led->priv, 5, NULL, 0, *((uint16_t*)&cameraOffsetU), 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	if(ret < 0)
		return ret;

	// TODO: implment endianess
	uint16_t wValue;
	// we compile with fno-strict-aliasing
	uint8_t* wValChar = (uint8_t*)&wValue;
	wValChar[0] = channels;
	wValChar[1] = currentU;
	while((ret = usbshm_writeControlTransfer(led->priv, 1, NULL, 0, wValue, timeU)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

void uvosled_disconnect(struct uvosled* led)
{
	usbshm_distroy(led->priv);
	free(led->priv);
	led->priv = NULL;
}
