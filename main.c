/**
* libeismultiplexer
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eismultiplexer.h"

void print_help(const char* progname)
{
	printf("usage: %s [OPERATION] [CHANNEL] [VALUE]\n", progname);
	printf("available operations: connect disconnect help\n");
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
			return CHANNEL_NONE;
		default:
			return CHANNEL_NONE;
	}
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

	if(argc != 4)
	{
		print_help(argv[0]);
		eismultiplexer_disconnect(&multiplexer);
		return 2;
	}

	channel_t channel =  char_to_channel(argv[2][0]);
	if(channel == CHANNEL_NONE)
	{
		printf("%c is not a valid channel", argv[2][0]);
		eismultiplexer_disconnect(&multiplexer);
		return 2;
	}
	
	if(strcmp(argv[1], "connect") == 0)
	{
		if(eismultiplexer_connect_channel(&multiplexer, channel))
		{
			printf("could not connect channel %c", argv[2][0]);
			eismultiplexer_disconnect(&multiplexer);
			return 3;
		}
	}
	else if(strcmp(argv[1], "disconnect") == 0)
	{
		if(eismultiplexer_disconnect_channel(&multiplexer, channel))
		{
			printf("could not connect channel %c", argv[2][0]);
			eismultiplexer_disconnect(&multiplexer);
			return 3;
		}
	}

	eismultiplexer_disconnect(&multiplexer);
	return 0;
}
