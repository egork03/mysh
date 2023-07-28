// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once
#include "token.h"

typedef struct command_node_s command_node_t;

typedef struct parser_s
{
	char const* str;
    char const* pos;

	token_t* t;     // last recognized token
	token_t* la;    // lookahead token

	command_node_t* cmd_root_node;

}
parser_t;

command_node_t* parse_command_line(char const* command_line);

