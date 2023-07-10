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

#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
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
	void* user_data;
	void (*dataCallback)(uint8_t request, unsigned char* data, size_t length, void* user_data);
};

void usbshm_distroy(struct usbshm* instance);

int usbshm_init(struct usbshm* instance, void (*dataCallback)(uint8_t request, unsigned char* data, size_t length, void* user_data), void* user_data);

bool usbshm_ready(struct usbshm* instance);

int usbshm_open(struct usbshm* instance, int vendorID, int productID, const unsigned char* serial);

bool usbshm_isOpen(struct usbshm* instance);

void usbshm_reset(struct usbshm* instance);

void usbshm_reopen(struct usbshm* instance);

int usbshm_writeControlTransfer(struct usbshm* instance, const uint8_t request, 
								char* buffer, const uint8_t length, 
								const uint16_t wValue, const uint16_t wIndex);

int usbshm_readControlTransfer(struct usbshm* instance, const uint8_t request, const uint16_t wValue, const uint16_t wIndex, const uint16_t length);

int usbshm_readControlTransferSync(struct usbshm* instance, const uint8_t request, const uint16_t wValue, const uint16_t wIndex, uint8_t* buffer, const uint16_t length);

