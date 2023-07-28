// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once

// context-independent lexer tokens
typedef enum token_type_e
{
	TOKEN_EOF=0,
	TOKEN_ERROR=1,
	TOKEN_COMMAND_CD=2,
	TOKEN_COMMAND_PWD=3,
	TOKEN_COMMAND_EXIT=4,
	TOKEN_PATH=5,
	TOKEN_WILDCARD=6,
	TOKEN_REDIRECTION_IN=7,
	TOKEN_REDIRECTION_OUT=8,
	TOKEN_PIPE=9,
	TOKEN_AND=10,
	TOKEN_OR=11
}
token_type_t;

unsigned int static const maxT = 12;


