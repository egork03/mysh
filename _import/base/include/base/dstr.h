// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once
#include "bool.h"

// dynamic string class
// supports assigning and appending of different string types such as
// string views and normal strings

typedef int  dstr_len_t;
typedef char dstr_chr_t;

typedef struct strd_s
{
	dstr_chr_t* ptr;
	dstr_len_t cap;
	dstr_len_t len;
}
dstr_t;

// Construction
void     dstr_init(dstr_t* this_p);
void     dstr_term(dstr_t* this_p);

// Attributes
bool    dstr_is_null(dstr_t const* this_p);
bool   dstr_is_empty(dstr_t const* this_p);
int      dstr_length(dstr_t const* this_p);

// Assign Methods
bool     dstr_assign_view(dstr_t* this_p, dstr_chr_t const* ptr, dstr_len_t len);
bool      dstr_assign_str(dstr_t* this_p, dstr_chr_t const* ptr);
bool     dstr_assign_dstr(dstr_t* this_p, dstr_t const* str);

// Append Methods
bool      dstr_append_chr(dstr_t* this_p, char c);
bool     dstr_append_view(dstr_t* this_p, dstr_chr_t const* ptr, dstr_len_t len);
bool      dstr_append_str(dstr_t* this_p, dstr_chr_t const* ptr);
bool     dstr_append_dstr(dstr_t* this_p, dstr_t const* str);

