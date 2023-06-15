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

#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <stdbool.h>

struct usbshm_priv;

enum {
	USBSHM_ERROR_AGAIN = -1,
	USBSHM_ERROR_PARAM = -2,
	USBSHM_ERROR_ERR = -3,
	USBSHM_ERROR_NOT_CONNECTED = -4,
};

struct usbshm {
	struct usbshm_priv* priv;
	int vendorID;
	int productID;
	char* productName;
	void (*dataCallback)(uint8_t request, unsigned char* data, size_t length);
};

void usbshm_distroy(struct usbshm* instance);

int usbshm_init(struct usbshm* instance, void (*dataCallback)(uint8_t request, unsigned char* data, size_t length));

bool usbshm_ready(struct usbshm* instance);

int usbshm_open(struct usbshm* instance, int vendorID, int productID, const char* productName);

bool usbshm_isOpen(struct usbshm* instance);

void usbshm_reset(struct usbshm* instance);

void usbshm_reopen(struct usbshm* instance);

int usbshm_writeControlTransfer(struct usbshm* instance, const uint8_t request, 
								char* buffer, const uint8_t length, 
								const uint16_t wValue, const uint16_t wIndex);

int usbshm_readControlTransfer(struct usbshm* instance, const uint8_t request, const uint8_t length);

