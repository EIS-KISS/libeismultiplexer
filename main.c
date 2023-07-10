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
#include <signal.h>
#include "eismultiplexer.h"
#include "options.h"

static channel_t char_to_channel(char ch)
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

static int process_commands(char** commands, size_t command_count, struct eismultiplexer* multiplexer, char* name)
{
	if(strcmp(commands[0], "clear") == 0)
	{
		eismultiplexer_disconnect_channel(multiplexer, CHANNEL_NONE);
	}
	else if(strcmp(commands[0], "write") == 0)
	{
		if(command_count < 3)
		{
			printf("Usage %s %s [ADDRESS] [VALUE]\n", name, commands[0]);
			return 2;
		}
		long address = strtol(commands[1], NULL, 10);
		long value = strtol(commands[2], NULL, 10);
		if(address > UINT16_MAX || address < 0 || value < 0 || value > UINT16_MAX)
		{
			puts("Value or address is out of range");
			return 2;
		}
		if(address % 2 == 1)
		{
			puts("Address must be a even number, 16 bits are written at a time");
			return 2;
		}
		eismultiplexer_write_eeprom(multiplexer, address, value);
	}
	else if(strcmp(commands[0], "read") == 0)
	{
		if(command_count < 2)
		{
			printf("Usage %s %s [ADDRESS] [LENGTH]\n", name, commands[0]);
			return 2;
		}
		long address = strtol(commands[1], NULL, 10);
		long length = 2;
		if(command_count > 2)
			length = strtol(commands[2], NULL, 10);
		if(address > UINT16_MAX || address < 0 || length < 0 || length > UINT16_MAX)
		{
			return 2;
			puts("Value or address is out of range");
		}
		if(address % 2 == 1)
		{
			puts("Address must be a even number, 16 bits are read at a time");
			return 2;
		}
		if(length % 2 == 1)
		{
			puts("Length must be a even number, 16 bits are read at a time");
			return 2;
		}

		for(uint16_t i = address; i - address < length; i+=2)
		{
			uint16_t value = eismultiplexer_read_eeprom(multiplexer, i);
			printf("%u: %u\n", i, value);
		}
	}
	else if(strcmp(commands[0], "connect_all") == 0)
	{
		eismultiplexer_connect_channel(multiplexer, 0xff);
	}
	else if(strcmp(commands[0], "get") == 0)
	{
		channel_t channels = eismultiplexer_get_connected(multiplexer);
		for(size_t i = 0; i < 7; ++i)
		{
			bool connected = channels & (1 << i);
			printf("Channel %c: %s\n", (char)('A'+i), connected ? "on" : "off");
		}
	}
	else
	{
		if(command_count != 2)
		{
			printf("Usage %s %s [CHANNEL]\n", name, commands[0]);
			return 2;
		}

		channel_t channel =  char_to_channel(commands[1][0]);
		if(channel == CHANNEL_NONE)
		{
			printf("%c is not a valid channel\n", commands[1][0]);
			return 2;
		}

		if(strcmp(commands[0], "connect") == 0)
		{
			if(eismultiplexer_connect_channel(multiplexer, channel))
			{
				printf("could not connect channel %c\n", commands[1][0]);
				return 3;
			}
		}
		else if(strcmp(commands[0], "disconnect") == 0)
		{
			if(eismultiplexer_disconnect_channel(multiplexer, channel))
			{
				printf("could not disconnect channel %c\n", commands[1][0]);
				return 3;
			}
		}
	}
	return 0;
}

enum
{
	INPUT_SUCESS = 0,
	INPUT_NO_INPUT,
	INPUT_TOO_LONG
};

static int get_input(char *prompt, char *buffer, size_t length)
{
	int ch, extra;

	if(prompt != NULL)
	{
		fputs(prompt, stdout);
		fflush(stdout);
	}
	if(fgets (buffer, length, stdin) == NULL)
		return INPUT_NO_INPUT;

	if(buffer[strlen(buffer)-1] != '\n')
	{
		extra = 0;
		while((ch = getchar()) != '\n' && ch != EOF)
			extra = 1;
		return (extra == 1) ? INPUT_TOO_LONG : INPUT_SUCESS;
	}

	buffer[strlen(buffer)-1] = '\0';
	return INPUT_SUCESS;
}


int main(int argc, char* argv[])
{
	struct config config = {};
	argp_parse(&argp, argc, argv, 0, 0, &config);

	if(config.command_count < 1 && !(config.interactive || config.pipe))
	{
		printf("A command is required\n");
		return 2;
	}
	
	struct eismultiplexer multiplexer;
	if(eismultiplexer_connect(&multiplexer, config.serialSet ? config.serial : 0))
	{
		char serialStr[5];
		snprintf(serialStr, sizeof(serialStr), "%04hu", config.serial);
		printf("Can not connect to EISmultiplexer device%s%s\n",
			   config.serialSet ? " with serial " : "", config.serialSet ? serialStr : "");
		return 1;
	}

	int ret = 0;

	if(config.interactive || config.pipe)
	{
		char buffer[256];
		while(true)
		{
			int ret = get_input(config.pipe ? NULL : "> ", buffer, sizeof(buffer));
			if(ret == INPUT_NO_INPUT)
			{
				break;
			}
			else if(ret != INPUT_SUCESS)
			{
				puts("Invalid command");
				continue;
			}

			char* commands[MAX_COMMANDS];
			char* command = strtok(buffer, " \t");
			size_t i = 0;
			while(command  && i < MAX_COMMANDS)
			{
				commands[i] = command;
				command = strtok(NULL, " \t");
				++i;
			}

			int cmdRet = process_commands(commands, i, &multiplexer, "");
			if(cmdRet == 0)
				puts("OK");
			else
				printf("ERR %i\n", cmdRet);
		}
	}
	else
	{
		eismultiplexer_set_led(&multiplexer, true);
		ret = process_commands(config.commands, config.command_count, &multiplexer, argv[0]);
		eismultiplexer_set_led(&multiplexer, false);
	}

	eismultiplexer_disconnect(&multiplexer);
	return ret;
}
