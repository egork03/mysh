// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once

#include "base/dstr.h"
#include "parser.token.h"

char const* token_type_to_str(token_type_t tt);

typedef struct token_s
{
	dstr_t token_text;
	token_type_t token_type;
}
token_t;

void token_init(token_t* t);
void token_term(token_t* t);

int token_compose(token_t* t, token_type_t type, char const* ptr, dstr_len_t len);


