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
