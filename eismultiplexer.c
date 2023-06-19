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
#include "eismultiplexer.h"
#include "usbshm.h"
#include <stdlib.h>
#include <time.h>

static void usleep(uint64_t microseconds)
{
	struct timespec ts;
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	nanosleep(&ts, NULL);
}

static void dataCallback(uint8_t request, unsigned char* data, size_t length, void* user_data)
{
	struct eismulitplexer* muliplexer = user_data;

	if(length >= 1)
		muliplexer->activeChannels = data[0];

	sem_post(&muliplexer->readSem);
}

int eismulitplexer_connect(struct eismulitplexer* muliplexer, int serial)
{
	int ret;
	muliplexer->priv = malloc(sizeof(*muliplexer->priv));
	if(!muliplexer->priv)
		return -1;
	
	ret = usbshm_init(muliplexer->priv, &dataCallback, muliplexer);
	if(ret)
		return -2;
	
	ret = usbshm_open(muliplexer->priv, 0xfe17, 0x07dc , NULL);
	if(ret)
		return -3;

	ret = sem_init(&muliplexer->readSem, 0, 0);
	if(ret)
		return -4;
	return 0;
}

int eismulitplexer_connect_channel_exclusive(struct eismulitplexer* muliplexer, channel_t channel)
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

int eismulitplexer_connect_channel(struct eismulitplexer* muliplexer, channel_t channel)
{
	channel_t channels = eismulitplexer_get_connected(muliplexer);
	channels |= channel;
	return eismulitplexer_connect_channel_exclusive(muliplexer, channels);
}

int eismulitplexer_disconnect_channel(struct eismulitplexer* muliplexer, channel_t channel)
{
	channel_t channels = eismulitplexer_get_connected(muliplexer);
	channels &= ~channel;
	return eismulitplexer_connect_channel_exclusive(muliplexer, channels);
}

channel_t eismulitplexer_get_connected(struct eismulitplexer* muliplexer)
{
	usbshm_readControlTransfer(muliplexer->priv, 3, 1);
	sem_wait(&muliplexer->readSem);
	return muliplexer->activeChannels;
}


int eismulitplexer_set_led(struct eismulitplexer* muliplexer, bool on)
{
	int ret;
	while((ret = usbshm_writeControlTransfer(muliplexer->priv, on, NULL, 0, 0, 0)) == USBSHM_ERROR_AGAIN)
		usleep(1000000);
	return ret;
}

void eismulitplexer_disconnect(struct eismulitplexer* muliplexer)
{
	usbshm_distroy(muliplexer->priv);
	free(muliplexer->priv);
	muliplexer->priv = NULL;
	sem_destroy(&muliplexer->readSem);
}
