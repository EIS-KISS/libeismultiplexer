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
#include <argp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_COMMANDS 8

const char *argp_program_version = "eismulitplexer_cli";
const char *argp_program_bug_address = "<carl@uvos.xyz>";
static char doc[] = "Applicaion to control eismultiplexer devices";
static char args_doc[] = "COMMAND";

static struct argp_option options[] =
{
  {"interactive",	'i', 0,				0,	"run in interactive mode" },
  {"pipe",			'p', 0,				0,	"run in pipe mode" },
  {"serial",		's', "[NUMMBER]",	0,	"serial number of device to connect to"},
  {"list",			'l', 0,				0,	"list commands"},
  {0}
};

struct command
{
	char* command;
};

struct config
{
	bool interactive;
	bool pipe;
	uint16_t serial;
	bool serialSet;
	char* commands[MAX_COMMANDS];
	size_t command_count;
	bool list_commands;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct config *config = (struct config*)(state->input);
	switch (key)
	{
	case 'i':
		config->interactive = true;
		break;
	case 's':
	{
		int serial = atoi(arg);
		if(serial < 0 || serial > UINT16_MAX )
		{
			argp_usage(state);
			break;
		}
		config->serial = serial;
		config->serialSet = true;
		break;
	}
	case 'p':
		config->pipe = true;
		break;
	case 'l':
		config->list_commands = true;
		break;
	case ARGP_KEY_ARG:
	{
		if(config->command_count+1 > MAX_COMMANDS)
		{
			argp_usage(state);
			break;
		}
		config->commands[config->command_count] = arg;
		++config->command_count;
		break;
	}
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};
