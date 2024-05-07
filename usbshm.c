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

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include "usbshm.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

static int object_counter = 0;
static struct libusb_context* context = NULL;

__attribute__ ((visibility ("hidden")))
static int usbshm_init(struct usbshm* instance)
{
	int ret = 0;
	if(object_counter == 0)
		ret = libusb_init(&context) < 0 ? USBSHM_ERROR_ERR : 0;
	if(ret >= 0)
	{
		++object_counter;
		instance->handle = NULL;
		instance->vendorID = 0;
		instance->productID = 0;
		instance->serial = NULL;
		instance->mutex =  malloc(sizeof(*(instance->mutex)));
		pthread_mutex_init(instance->mutex, NULL);
	}

	return ret < 0 ? USBSHM_ERROR_ERR : 0;
}

__attribute__ ((visibility ("hidden")))
static void usbshm_exit(void)
{
	if(--object_counter == 0)
	{
		libusb_exit(context);
		context = NULL;
	}
}

__attribute__ ((visibility ("hidden")))
void usbshm_close(struct usbshm* instance)
{
	if(!instance->handle)
		return;
	pthread_mutex_lock(instance->mutex);
	libusb_close(instance->handle);
	if(instance->serial)
		free(instance->serial);
	instance->handle = NULL;
	usbshm_exit();
	pthread_mutex_unlock(instance->mutex);
	pthread_mutex_destroy(instance->mutex);
}

__attribute__ ((visibility ("hidden")))
bool usbshm_is_open(struct usbshm* instance)
{
	return instance->handle != NULL;
}

__attribute__ ((visibility ("hidden")))
int usbshm_open(struct usbshm* instance, int vendorID, int productID, const unsigned char* serial)
{
	int ret = usbshm_init(instance);
	if(ret < 0)
		return ret;

	pthread_mutex_lock(instance->mutex);
	libusb_device** list;
	int count = libusb_get_device_list(context, &list);
	int errorCode = 0;
	if(count > 0)
	{
		struct libusb_device_descriptor desc = {0};
		for(int i = 0; i < count; ++i)
		{
			libusb_get_device_descriptor(list[i], &desc);
			if(desc.idVendor == vendorID && desc.idProduct == productID)
			{
				errorCode = libusb_open(list[i], &instance->handle) < 0 ? USBSHM_ERROR_ERR : 0;
				if(errorCode != USBSHM_ERROR_ERR && instance->handle)
				{
					if(serial)
					{
						size_t len = strlen((const char*)serial)+1;
						unsigned char* buffer = malloc(len);
						buffer[0] = '\0';
						libusb_get_string_descriptor_ascii(instance->handle, desc.iSerialNumber, buffer, len);
						int cmp = strcmp((const char*)serial, (const char*)buffer);
						free(buffer);
						if(cmp != 0)
						{
							libusb_close(instance->handle);
							instance->handle = NULL;
							continue;
						}
					}
					break;
				}
				else
				{
					instance->handle = NULL;
				}
			}
		}
	}
	else
	{
		printf("Can not list devices\n");
		pthread_mutex_unlock(instance->mutex);
		return USBSHM_ERROR_ERR;
	}

	pthread_mutex_unlock(instance->mutex);

	if(usbshm_is_open(instance))
	{
		instance->vendorID = vendorID;
		instance->productID = productID;
		if(serial)
		{
			instance->serial = calloc(strlen((char*)serial), 1);
			memcpy(instance->serial, serial, strlen((char*)serial));
		}
		libusb_set_auto_detach_kernel_driver(instance->handle, true);
	}
	else
	{
		printf("Opening usb device failed\n");
		errorCode = USBSHM_ERROR_ERR;
	}

	libusb_free_device_list(list, count);
	return errorCode;
}

__attribute__ ((visibility ("hidden")))
bool usbshm_usbshm_is_open(struct usbshm* instance)
{
	pthread_mutex_lock(instance->mutex);
	bool ret = instance->handle != NULL;
	pthread_mutex_unlock(instance->mutex);
	return ret;
}

__attribute__ ((visibility ("hidden")))
void usbshm_reset(struct usbshm* instance)
{
	if(!instance->handle)
		return;
	pthread_mutex_lock(instance->mutex);
	libusb_reset_device(instance->handle);
	pthread_mutex_unlock(instance->mutex);
}

__attribute__ ((visibility ("hidden")))
void usbshm_reopen(struct usbshm* instance)
{
	usbshm_reset(instance);
	libusb_close(instance->handle);
	char* serial = strdup((char*)instance->serial);
	free(instance->serial);
	usbshm_open(instance, instance->vendorID, instance->productID, (unsigned char*)serial);
}

__attribute__ ((visibility ("hidden")))
int usbshm_write_control_transfer(struct usbshm* instance, const uint8_t request,
								uint8_t* buffer, const uint8_t length,
								const uint16_t wValue, const uint16_t wIndex)
{
	if(!instance->handle)
		return USBSHM_ERROR_NOT_CONNECTED;
	if(!usbshm_is_open(instance))
		return USBSHM_ERROR_NOT_CONNECTED;
	if(length > 8)
		return USBSHM_ERROR_PARAM;
	pthread_mutex_lock(instance->mutex);
	int ret = libusb_control_transfer(instance->handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
									  request, wValue, wIndex, buffer, length, 2000);
	pthread_mutex_unlock(instance->mutex);

	if(ret == LIBUSB_ERROR_TIMEOUT)
		ret = USBSHM_ERROR_TIMEOUT;
	else if(ret < 0 )
		ret = USBSHM_ERROR_ERR;

	return ret;
}

__attribute__ ((visibility ("hidden")))
int usbshm_read_control_transfer(struct usbshm* instance, const uint8_t request, const uint16_t wValue, const uint16_t wIndex, uint8_t* buffer, const uint16_t length)
{
	if(!instance->handle)
		return USBSHM_ERROR_NOT_CONNECTED;
	pthread_mutex_lock(instance->mutex);
	int ret = libusb_control_transfer(instance->handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
									 request, wValue, wIndex, buffer, length, 2000);
	pthread_mutex_unlock(instance->mutex);

	if(ret == LIBUSB_ERROR_TIMEOUT)
		ret = USBSHM_ERROR_TIMEOUT;
	else if(ret < 0 )
		ret = USBSHM_ERROR_ERR;

	return ret;
}
