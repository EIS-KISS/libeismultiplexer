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
	CHANNEL_A = (1 << 1),
	CHANNEL_B = (1 << 2),
	CHANNEL_C = (1 << 3),
	CHANNEL_D = (1 << 4),
	CHANNEL_E = (1 << 5),
	CHANNEL_F = (1 << 6),
	CHANNEL_G = (1 << 7),
	CHANNEL_NONE = 0,
} channel_t;

struct eismultiplexer {
	struct usbshm* priv;
	sem_t readSem;
	channel_t activeChannels;
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

#ifdef __cplusplus
}
#endif

/**
....
* @}
*/
