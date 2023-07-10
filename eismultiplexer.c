/* * Copyright (c) 2023 Carl Klemm <carl@uvos.xyz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *  * Neither the name of %ORGANIZATION% nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include "eismultiplexer.h"
#include "usbshm.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static void usleep(uint64_t microseconds)
{
	struct timespec ts;
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	nanosleep(&ts, NULL);
}

int eismultiplexer_connect(struct eismultiplexer* muliplexer, uint16_t serial)
{
	int ret;
	muliplexer->priv = malloc(sizeof(*muliplexer->priv));
	if(!muliplexer->priv)
		return -1;
	
	ret = usbshm_init(muliplexer->priv, NULL, muliplexer);
	if(ret)
		return -2;

	unsigned char serialStr[5];
	snprintf((char*)serialStr, sizeof(serialStr), "%04hu", serial);

	ret = usbshm_open(muliplexer->priv, 0xfe17, 0x07dc , serial == 0 ? NULL : serialStr);
	if(ret)
		return -3;
	return 0;
}

int eismultiplexer_connect_channel_exclusive(struct eismultiplexer* muliplexer, channel_t channel)
{
	uint16_t wValue;
	// we compile with fno-strict-aliasing
	uint8_t* wValChar = (uint8_t*)&wValue;
	wValChar[0] = channel;
	wValChar[1] = 0;
	int ret;
	while((ret = usbshm_writeControlTransfer(muliplexer->priv, 2, NULL, 0, wValue, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

int eismultiplexer_connect_channel(struct eismultiplexer* muliplexer, channel_t channel)
{
	channel_t channels = eismultiplexer_get_connected(muliplexer);
	channels |= channel;
	return eismultiplexer_connect_channel_exclusive(muliplexer, channels);
}

int eismultiplexer_disconnect_channel(struct eismultiplexer* muliplexer, channel_t channel)
{
	channel_t channels = CHANNEL_NONE;
	if(channel != CHANNEL_NONE)
	{
		channels = eismultiplexer_get_connected(muliplexer);
		channels &= ~channel;
	}
	return eismultiplexer_connect_channel_exclusive(muliplexer, channels);
}

channel_t eismultiplexer_get_connected(struct eismultiplexer* muliplexer)
{
	uint8_t buffer[2] = {};
	usbshm_readControlTransferSync(muliplexer->priv, 3, 0, 0, buffer, 1);
	return buffer[0];
}

int eismultiplexer_set_led(struct eismultiplexer* muliplexer, bool on)
{
	int ret;
	while((ret = usbshm_writeControlTransfer(muliplexer->priv, on, NULL, 0, 0, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

int eismultiplexer_write_eeprom(struct eismultiplexer* muliplexer, uint16_t addr, uint16_t value)
{
	int ret;
	while((ret = usbshm_writeControlTransfer(muliplexer->priv, 5, NULL, 0, value, addr)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

uint16_t eismultiplexer_read_eeprom(struct eismultiplexer* muliplexer, uint16_t addr)
{
	uint8_t buffer[2] = {};
	usbshm_readControlTransferSync(muliplexer->priv, 4, 0, addr, buffer, 2);
	return *((uint16_t*)buffer);
}

void eismultiplexer_disconnect(struct eismultiplexer* muliplexer)
{
	usbshm_distroy(muliplexer->priv);
	free(muliplexer->priv);
	muliplexer->priv = NULL;
}
