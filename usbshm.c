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

#include "usbshm.h"

#include <stdint.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <errno.h>
#include <unistd.h>

struct usbshm_priv
{
	libusb_device_handle* handle;
	struct libusb_transfer* transfer;
	unsigned char* buffer;
};

static unsigned objectCounter = 0;
static struct timeval timeout = {1, 0};
static pthread_mutex_t *libusbDataMutex;
static pthread_t libusbThread;
static atomic_bool threadStop;

static void usbshm_transferCallBack(struct libusb_transfer *transfer);

void usbshm_distroy(struct usbshm* instance)
{
	while(!usbshm_ready(instance))
		sleep(1);
	free(instance->priv->transfer);
	libusb_close(instance->priv->handle);
	free(instance->priv->buffer);
	free(instance->priv);
	if(instance->serial)
		free(instance->serial);
	if(--objectCounter == 0)
	{
		threadStop = true;
		pthread_join(libusbThread, NULL);
		libusb_exit(NULL);
		pthread_mutex_destroy(libusbDataMutex);
	}
}

static void* usbshm_libusbPoll(void* arg)
{
	while(!threadStop)
	{
		libusb_handle_events_timeout_completed(NULL, &timeout, NULL);
	}
	return NULL;
}

bool usbshm_isOpen(struct usbshm* instance)
{
	return instance->priv->handle != NULL;
}

int usbshm_init(struct usbshm* instance, void (*dataCallback)(uint8_t request, unsigned char* data, size_t length, void* user_data), void* user_data)
{
	int ret=0;
	instance->priv = malloc(sizeof(*instance->priv));
	instance->priv->handle = NULL;
	instance->priv->transfer = NULL;
	instance->priv->buffer = NULL;
	instance->vendorID = 0;
	instance->productID = 0;
	instance->serial = NULL;
	instance->dataCallback = dataCallback;
	instance->user_data = user_data;
	if(objectCounter == 0)
	{
		ret = libusb_init(NULL) < 0 ? USBSHM_ERROR_ERR : 0;
		libusbDataMutex = malloc(sizeof(*libusbDataMutex));
		pthread_mutex_init(libusbDataMutex, NULL);
		pthread_create(&libusbThread, NULL, &usbshm_libusbPoll, NULL);
	}
	if(ret == 0)
		objectCounter++;
	return ret < 0 ? USBSHM_ERROR_ERR : 0;
}

bool usbshm_ready(struct usbshm* instance)
{
	return instance->priv->transfer == NULL;
}

int usbshm_open(struct usbshm* instance, int vendorID, int productID, const unsigned char* serial)
{
	instance->priv->handle = NULL;
	pthread_mutex_lock(libusbDataMutex);
	libusb_device** list;
	int count = libusb_get_device_list(NULL, &list);
	int errorCode = 0;
	if( count > 0)
	{
		struct libusb_device_descriptor desc = {0};
		for(int i = 0; i < count; ++i)
		{
			libusb_get_device_descriptor(list[i], &desc);
			if(desc.idVendor == vendorID && desc.idProduct == productID)
			{
				errorCode = libusb_open(list[i], &instance->priv->handle) < 0 ? USBSHM_ERROR_ERR : 0;
				if(errorCode != USBSHM_ERROR_ERR && instance->priv->handle)
				{
					if(serial)
					{
						size_t len = strlen((const char*)serial)+1;
						unsigned char* buffer = malloc(len);
						buffer[0] = '\0';
						libusb_get_string_descriptor_ascii(instance->priv->handle, desc.iSerialNumber, buffer, len);
						int cmp = strcmp((const char*)serial, (const char*)buffer);
						free(buffer);
						if(cmp != 0)
						{
							libusb_close(instance->priv->handle);
							instance->priv->handle = NULL;
							continue;
						}
					}
					break;
				}
				else
				{
					instance->priv->handle = NULL;
				}
			}
		}
	}
	else
	{
		printf("Can not list devices\n");
		pthread_mutex_unlock(libusbDataMutex);
		return USBSHM_ERROR_ERR;
	}

	if(usbshm_isOpen(instance))
	{
		instance->vendorID = vendorID;
		instance->productID = productID;
		if(serial)
		{
			instance->serial = calloc(strlen((char*)serial), 1);
			memcpy(instance->serial, serial, strlen((char*)serial));
		}
		libusb_set_auto_detach_kernel_driver(instance->priv->handle, true);
	}
	else
	{
		printf("Opening usb device failed\n");
		errorCode = USBSHM_ERROR_ERR;
	}

	libusb_free_device_list(list, count);
	pthread_mutex_unlock(libusbDataMutex);
	return errorCode;
}

bool usbshm_usbshm_isOpen(struct usbshm* instance)
{
	pthread_mutex_lock(libusbDataMutex);
	bool ret = instance->priv->handle != NULL;
	pthread_mutex_unlock(libusbDataMutex);
	return ret;
}

void usbshm_reset(struct usbshm* instance)
{
	pthread_mutex_lock(libusbDataMutex);
	libusb_reset_device(instance->priv->handle);
	pthread_mutex_unlock(libusbDataMutex);
}

void usbshm_reopen(struct usbshm* instance)
{
	usbshm_reset(instance);
	libusb_close(instance->priv->handle);
	usbshm_open(instance, instance->vendorID, instance->productID, instance->serial);
}

int usbshm_writeControlTransfer(struct usbshm* instance, const uint8_t request,
								char* buffer, const uint8_t length,
								const uint16_t wValue, const uint16_t wIndex)
{
	if(!usbshm_isOpen(instance))
		return USBSHM_ERROR_NOT_CONNECTED;
	if(length > 8)
		return USBSHM_ERROR_PARAM;
	if(instance->priv->transfer == NULL)
	{
		pthread_mutex_lock(libusbDataMutex);
		instance->priv->buffer = malloc(length+8);
		libusb_fill_control_setup(instance->priv->buffer, 
								  LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, 
								  request, wValue, wIndex, length );
		for(uint8_t i = 0; i < length; ++i) 
			instance->priv->buffer[i+8] = buffer[i];
		instance->priv->transfer = libusb_alloc_transfer(0);
		libusb_fill_control_transfer(instance->priv->transfer, instance->priv->handle, instance->priv->buffer, 
									 &usbshm_transferCallBack, instance, 100);
		int ret = libusb_submit_transfer(instance->priv->transfer);
		if(ret < 0)
		{
			free(buffer);
			libusb_free_transfer(instance->priv->transfer);
			instance->priv->transfer = NULL;
			if(ret == LIBUSB_ERROR_NO_DEVICE) 
				usbshm_reopen(instance);
			pthread_mutex_unlock(libusbDataMutex);
		}
		return ret < 0 ? USBSHM_ERROR_ERR : 0;
	}
	else return USBSHM_ERROR_AGAIN;
}

int usbshm_readControlTransfer(struct usbshm* instance, const uint8_t request, const uint16_t wValue, const uint16_t wIndex, const uint16_t length)
{
	if(!usbshm_isOpen(instance))
		return USBSHM_ERROR_NOT_CONNECTED;
	if(length > 8) 
		return USBSHM_ERROR_PARAM;
	if(instance->priv->transfer == NULL)
	{
		pthread_mutex_lock(libusbDataMutex);
		instance->priv->buffer = malloc(length+8);
		libusb_fill_control_setup(instance->priv->buffer, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
			request, wValue, wIndex, length);
		instance->priv->transfer = libusb_alloc_transfer(0);
		libusb_fill_control_transfer(instance->priv->transfer, instance->priv->handle, instance->priv->buffer,
			&usbshm_transferCallBack, instance, 100);
		int ret = libusb_submit_transfer(instance->priv->transfer);
		if(ret < 0)
		{
			free(instance->priv->buffer);
			libusb_free_transfer(instance->priv->transfer);
			instance->priv->transfer = NULL;
			if(ret == LIBUSB_ERROR_NO_DEVICE) 
				usbshm_reopen(instance);
			pthread_mutex_unlock(libusbDataMutex);
		}
		return ret < 0 ? USBSHM_ERROR_ERR : 0;
	}
	else return USBSHM_ERROR_AGAIN;
}

static void usbshm_transferCallBack(struct libusb_transfer *transfer)
{
	struct usbshm* context = (struct usbshm*)transfer->user_data;
	if(transfer->status != LIBUSB_TRANSFER_COMPLETED)
	{
		printf("Usb transfer failed with %d\n", transfer->status);
	}
	else if(transfer->type == LIBUSB_TRANSFER_TYPE_CONTROL) 
	{
		if(context->dataCallback && transfer->length-8 >= transfer->actual_length) 
			context->dataCallback(transfer->buffer[1], transfer->buffer+8, transfer->actual_length, context->user_data);
	}
	free(context->priv->buffer);
	libusb_free_transfer(context->priv->transfer);
	context->priv->transfer = NULL;
	pthread_mutex_unlock(libusbDataMutex);
}

int usbshm_readControlTransferSync(struct usbshm* instance, const uint8_t request, const uint16_t wValue, const uint16_t wIndex, uint8_t* buffer, const uint16_t length)
{
	pthread_mutex_lock(libusbDataMutex);
	int ret = libusb_control_transfer(instance->priv->handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
									 request, wValue, wIndex, buffer, length, 2000);
	pthread_mutex_unlock(libusbDataMutex);
	return ret < 0 ? USBSHM_ERROR_ERR : 0;
}
