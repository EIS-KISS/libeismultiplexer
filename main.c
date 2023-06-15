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
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "uvosled.h"

void print_help(const char* progname)
{
	printf("usage: %s [OPERATION] [CHANNEL] [VALUE]\n", progname);
	printf("available operations: set on off help\n");
}

int main(int argc, char* argv[])
{
	if(argc < 2 || strcmp (argv[1], "help") == 0)
	{
		print_help(argv[0]);
		return 0;
	} 
	
	struct uvosled led;
	
	if(uvosled_connect(&led))
	{
		printf("Can not connect to UVOS usbled device\n");
		return -1;
	}
	
	if(strcmp(argv[1], "on") == 0)
	{
		if(uvosled_poweron(&led))
			printf("cant power on \n");
	}
	else if(strcmp(argv[1], "off") == 0)
	{
		uvosled_set_current(&led, 0xff , 0);
		uvosled_poweroff(&led);
	}
	else if(strcmp(argv[1], "set") == 0)
	{
		if(argc != 4)
		{
			printf("usage: %s set [CHANNEL] [VALUE]\n", argv[0]);
			uvosled_poweroff(&led);
			return 0;
		}
		
		uint8_t mask = 0;
		
		switch(argv[2][0])
		{
			case 'a':
			case 'A':
			case '1':
				mask = CHANNEL_A;
				break;
			case 'b':
			case 'B':
			case '2':
				mask = CHANNEL_B;
				break;
			case 'c':
			case 'C':
			case '3':
				mask = CHANNEL_C;
				break;
			case 'd':
			case 'D':
			case '4':
				mask = CHANNEL_D;
				break;
			default:
 				printf("A channel must be selected out of A, B, C or D\n");
		}
		
		if(mask)
		{
			char* end;
			float value = strtof(argv[3], &end);
			if(isnan(value) || value < 0 || value > 1.0f)
				printf("A channel value between 0 and 1.0 must be supplied\n");
			else if(uvosled_set_current(&led, mask, value))
				printf("Failed to set current\n");
		}
	}

	uvosled_disconnect(&led);
	return 0;
}
