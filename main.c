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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eismultiplexer.h"

void print_help(const char* progname)
{
	printf("usage: %s [OPERATION] [CHANNEL] [VALUE]\n", progname);
	printf("available operations: connect disconnect clear connect_all get help\n");
}

channel_t char_to_channel(char ch)
{
	switch(ch)
	{
		case 'a':
		case 'A':
		case '1':
			return CHANNEL_A;
		case 'b':
		case 'B':
		case '2':
			return CHANNEL_B;
		case 'c':
		case 'C':
		case '3':
			return CHANNEL_C;
		case 'd':
		case 'D':
		case '4':
			return CHANNEL_D;
		case 'e':
		case 'E':
		case '5':
			return CHANNEL_E;
		case 'f':
		case 'F':
		case '6':
			return CHANNEL_F;
		case 'g':
		case 'G':
		case '7':
			return CHANNEL_G;
		default:
			return CHANNEL_NONE;
	}
}


void disconnect(struct eismultiplexer* multiplexer)
{
	eismultiplexer_set_led(multiplexer, false);
	eismultiplexer_disconnect(multiplexer);
}


int main(int argc, char* argv[])
{
	if(argc < 2 || strcmp(argv[1], "help") == 0)
	{
		print_help(argv[0]);
		return 0;
	} 
	
	struct eismultiplexer multiplexer;
	
	if(eismultiplexer_connect(&multiplexer, 0))
	{
		printf("Can not connect to EISmultiplexer device\n");
		return 1;
	}

	eismultiplexer_set_led(&multiplexer, true);

	if(strcmp(argv[1], "clear") == 0)
	{
		eismultiplexer_disconnect_channel(&multiplexer, CHANNEL_NONE);
	}
	else if(strcmp(argv[1], "write") == 0)
	{
		eismultiplexer_write_eeprom(&multiplexer, 0, 42);
	}
	else if(strcmp(argv[1], "read") == 0)
	{
		uint16_t value = eismultiplexer_read_eeprom(&multiplexer, 0);
		printf("0: %u\n", value);
	}
	else if(strcmp(argv[1], "connect_all") == 0)
	{
		eismultiplexer_connect_channel(&multiplexer, 0xff);
	}
	else if(strcmp(argv[1], "get") == 0)
	{
		channel_t channels = eismultiplexer_get_connected(&multiplexer);
		printf("%d\n", channels);
		for(size_t i = 0; i < 7; ++i)
		{
			bool connected = channels & (1 << i);
			printf("Channel %c: %s\n", (char)('A'+i), connected ? "on" : "off");
		}
	}
	else
	{
		if(argc != 3)
		{
			printf("Usage %s %s [CHANNEL]\n", argv[0], argv[1]);
			disconnect(&multiplexer);
			return 2;
		}

		channel_t channel =  char_to_channel(argv[2][0]);
		if(channel == CHANNEL_NONE)
		{
			printf("%c is not a valid channel\n", argv[2][0]);
			disconnect(&multiplexer);
			return 2;
		}

		if(strcmp(argv[1], "connect") == 0)
		{
			if(eismultiplexer_connect_channel(&multiplexer, channel))
			{
				printf("could not connect channel %c\n", argv[2][0]);
				disconnect(&multiplexer);
				return 3;
			}
		}
		else if(strcmp(argv[1], "disconnect") == 0)
		{
			if(eismultiplexer_disconnect_channel(&multiplexer, channel))
			{
				printf("could not disconnect channel %c\n", argv[2][0]);
				disconnect(&multiplexer);
				return 3;
			}
		}
	}

	disconnect(&multiplexer);
	return 0;
}
