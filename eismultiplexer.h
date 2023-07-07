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

#pragma once

#include <stdint.h>
#include <semaphore.h>
#include <stdbool.h>

/**
Api to controll EISmultiplexer devices.
* @defgroup API User API
* This api allows you to controll the EISmultiplexer device.
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CHANNEL_A = (1 << 0),
	CHANNEL_B = (1 << 1),
	CHANNEL_C = (1 << 2),
	CHANNEL_D = (1 << 3),
	CHANNEL_E = (1 << 4),
	CHANNEL_F = (1 << 5),
	CHANNEL_G = (1 << 6),
	CHANNEL_NONE = 0,
} channel_t;

struct eismultiplexer {
	struct usbshm* priv;
};

/**
 * @brief Attempts to connect to a EISmultiplexer device and initalizes a eismultiplexer struct
 * @param muliplexer pointer to a eismultiplexer struct to initalize
 * @param serial The serial number of the device to connect to, or 0 for any
 * @return 0 on sucess and < 0 on failure
 */
int eismultiplexer_connect(struct eismultiplexer* muliplexer, int serial);

/**
 * @brief Conects the given channel(s) to the common inputs
 * @param muliplexer pointer to a eismultiplexer struct
 * @param channel A channel to connect, multiple channels can be specified by or'ing together the chanel flags e.g. (CHANNEL_A | CHANNEL_B)
 * @return 0 on sucess and < 0 on failure
 */
int eismultiplexer_connect_channel(struct eismultiplexer* muliplexer, channel_t channel);

/**
 * @brief Conects the given channel(s) to the common inputs disconnecting all others
 * @param muliplexer pointer to a eismultiplexer struct
 * @param channel A channel to connect, multiple channels can be specified by or'ing together the chanel flags e.g. (CHANNEL_A | CHANNEL_B)
 * @return 0 on sucess and < 0 on failure
 */
int eismultiplexer_connect_channel_exclusive(struct eismultiplexer* muliplexer, channel_t channel);

/**
 * @brief Disconnect the given channel(s) to the common inputs disconnecting all others
 * @param muliplexer pointer to a eismultiplexer struct
 * @param channel A channel to connect, multiple channels can be specified by or'ing together the chanel flags e.g. (CHANNEL_A | CHANNEL_B)
 * All channels can be dissconnected by passing CHANNEL_NONE
 * @return 0 on sucess and < 0 on failure
 */
int eismultiplexer_disconnect_channel(struct eismultiplexer* muliplexer, channel_t channel);

/**
 * @brief Returns the channels currently connected
 * @param muliplexer pointer to a eismultiplexer struct
 * @return channels connected as a bitfield
 */
channel_t eismultiplexer_get_connected(struct eismultiplexer* muliplexer);

/**
 * @brief Turns the led on the pcb on or off
 * @param muliplexer pointer to a eismultiplexer struct
 * @param on true to turn the led on, false to turn it off
 * @return 0 on sucess and < 0 on failure
 */
int eismultiplexer_set_led(struct eismultiplexer* muliplexer, bool on);

/**
 * @brief Disconnects from the eismultiplexer
 */
void eismultiplexer_disconnect(struct eismultiplexer* muliplexer);

int eismultiplexer_write_eeprom(struct eismultiplexer* muliplexer, uint16_t addr, uint16_t value);
uint16_t eismultiplexer_read_eeprom(struct eismultiplexer* muliplexer, uint16_t addr);

#ifdef __cplusplus
}
#endif

/**
....
* @}
*/
