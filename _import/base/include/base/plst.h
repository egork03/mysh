// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once
#include "bool.h"

// pointer list class

typedef int   plst_len_t;
typedef void* plst_item_t;
typedef unsigned plst_item_size_t;
typedef void (*plst_item_term_func_t)(plst_item_t* this_item);

typedef struct plst_s
{
	plst_item_t* ptr;
	plst_len_t cap;
	plst_len_t len;
}
plst_t;

// Construction
void     plst_init(plst_t* this_p);
void     plst_term(plst_t* this_p, plst_item_term_func_t item_term_func);

// Attributes
bool    plst_is_null(plst_t const* this_p);
bool   plst_is_empty(plst_t const* this_p);
int      plst_length(plst_t const* this_p);

// Methods
bool         plst_append(plst_t* this_p, plst_item_t item);
bool    plst_append_zero(plst_t* this_p);
bool plst_append_copy_from_view(plst_t* this_p, char const* str, int str_len);
bool  plst_append_copy_from_str(plst_t* this_p, char const* str);
