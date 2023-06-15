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

#include "usbshm.h"

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
	free(instance->productName);
	free(instance->priv);
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

int usbshm_init(struct usbshm* instance, void (*dataCallback)(uint8_t request, unsigned char* data, size_t length))
{
	int ret=0;
	instance->priv = malloc(sizeof(*instance->priv));
	instance->priv->handle = NULL;
	instance->priv->transfer = NULL;
	instance->priv->buffer = NULL;
	instance->vendorID = 0;
	instance->productID = 0;
	instance->productName = NULL;
	instance->dataCallback = dataCallback;
	if(objectCounter == 0)
	{
		printf("Usb Init\n"); 
		ret = libusb_init(NULL) < 0 ? USBSHM_ERROR_ERR : 0;
		libusbDataMutex = malloc(sizeof(*libusbDataMutex));
		pthread_mutex_init(libusbDataMutex, NULL);
		//libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
		pthread_create(&libusbThread, NULL, &usbshm_libusbPoll, NULL);
	}
	if(ret == 0) objectCounter++;
	return ret;
}

bool usbshm_ready(struct usbshm* instance)
{
	return instance->priv->transfer == NULL;
}

int usbshm_open(struct usbshm* instance, int vendorID, int productID, const char* productName)
{
	instance->priv->handle = NULL;
	pthread_mutex_lock(libusbDataMutex);
	printf("Listing Devices\n");
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
				break;
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
		if(productName)
		{
			instance->productName = malloc(strlen(productName)+1);
			strcpy(instance->productName, productName);
		}
		else
		{
			instance->productName = NULL;
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
	return instance->priv->handle != NULL;
}

void usbshm_reset(struct usbshm* instance)
{
	printf("Usb transfer failed with %u\n", instance->priv->transfer->status);
	libusb_reset_device(instance->priv->handle);
}

void usbshm_reopen(struct usbshm* instance)
{
	libusb_close(instance->priv->handle);
	usbshm_open(instance, instance->vendorID, instance->productID, instance->productName);
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
		pthread_mutex_unlock(libusbDataMutex);
		if(ret < 0)
		{
			free(buffer);
			libusb_free_transfer(instance->priv->transfer);
			instance->priv->transfer = NULL;
			if(ret == LIBUSB_ERROR_NO_DEVICE) 
				usbshm_reopen(instance);
		}
		return ret < 0 ? USBSHM_ERROR_ERR : 0;
	}
	else return USBSHM_ERROR_AGAIN;
}

int usbshm_readControlTransfer(struct usbshm* instance, const uint8_t request, const uint8_t length)
{
	if(!usbshm_isOpen(instance))
		return USBSHM_ERROR_NOT_CONNECTED;
	if(length > 8) 
		return USBSHM_ERROR_PARAM;
	if(instance->priv->transfer == NULL)
	{
		pthread_mutex_lock(libusbDataMutex);
		instance->priv->buffer = malloc(length+8);
		libusb_fill_control_setup(instance->priv->buffer, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, request, request,request,length);
		instance->priv->transfer = libusb_alloc_transfer(0);
		libusb_fill_control_transfer(instance->priv->transfer, instance->priv->handle, instance->priv->buffer, &usbshm_transferCallBack, instance, 100);
		int ret = libusb_submit_transfer(instance->priv->transfer);
		pthread_mutex_unlock(libusbDataMutex);
		if(ret < 0)
		{
			free(instance->priv->buffer);
			libusb_free_transfer(instance->priv->transfer);
			instance->priv->transfer = NULL;
			if(ret == LIBUSB_ERROR_NO_DEVICE) 
				usbshm_reopen(instance);
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
			context->dataCallback(transfer->buffer[1], transfer->buffer+8, transfer->actual_length);
	}
	free(context->priv->buffer);
	libusb_free_transfer(context->priv->transfer);
	context->priv->transfer = NULL;
}
