// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

// dynamic list class of specified items

#pragma once
#include "bool.h"

typedef int   dlst_len_t;
typedef void dlst_item_t;
typedef unsigned dlst_item_size_t;
typedef void (*dlst_item_term_func_t)(dlst_item_t* this_item);

typedef struct dlst_s
{
	dlst_item_t* ptr;
	dlst_len_t cap;
	dlst_len_t len;
	dlst_item_size_t item_size;
}
dlst_t;

// Construction
void     dlst_init(dlst_t* this_p, dlst_item_size_t item_size);
void     dlst_term(dlst_t* this_p, dlst_item_term_func_t item_term_func);

// Attributes
bool    dlst_is_null(dlst_t const* this_p);
bool   dlst_is_empty(dlst_t const* this_p);
int      dlst_length(dlst_t const* this_p);

// Methods
dlst_item_t*  dlst_at(dlst_t* this_p, dlst_len_t index);
bool      dlst_append(dlst_t* this_p, dlst_item_t* item);

