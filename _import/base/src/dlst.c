// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "dlst.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool dlst_internal_integrity_chk(dlst_t const* this_p);
static bool dlst_internal_realloc(dlst_t* this_p, dlst_len_t len);

void dlst_init(dlst_t* this_p, dlst_item_size_t item_size)
{ 
	this_p->ptr = 0; 
	this_p->len = 0; 
	this_p->cap = 0; 
	this_p->item_size = item_size;
}

void dlst_term(dlst_t* this_p, dlst_item_term_func_t item_term_func)
{
	if (this_p->ptr != 0)
	{
		if (item_term_func)
		{
			for (dlst_len_t i = 0; i < this_p->len; ++i)
			{
				dlst_item_t* ptr = (unsigned char*)this_p->ptr + (i * this_p->item_size);
				item_term_func(ptr);
			}
		}

		// protection against double free
		dlst_item_t* p = this_p->ptr;
		this_p->ptr = 0;
		free(p);
	}
}

bool dlst_is_null(dlst_t const* this_p)
{ 
	return this_p->ptr == 0; 
}

bool dlst_is_empty(dlst_t const* this_p)
{
	return this_p->len == 0;
}

int dlst_length(dlst_t const* this_p)
{
	return this_p->len;
}

dlst_item_t* dlst_at(dlst_t* this_p, dlst_len_t index)
{
	dlst_item_t* item = (unsigned char*)this_p->ptr + (index * this_p->item_size);
	return item;
}

bool dlst_append(dlst_t* this_p, dlst_item_t* item)
{
	if (!dlst_internal_integrity_chk(this_p))
		return false;

	dlst_len_t avail = this_p->cap - 1;
	if (avail >= this_p->len)
	{
		dlst_item_t* ptr = (unsigned char*)this_p->ptr + (this_p->len * this_p->item_size);
		memcpy(ptr, item, this_p->item_size);
		++(this_p->len);
		return true;
	}

 	dlst_len_t new_len = this_p->len + 1;
	if (!dlst_internal_realloc(this_p, new_len))
		return false;

	dlst_item_t* ptr = (unsigned char*)this_p->ptr + (this_p->len * this_p->item_size);
	memcpy(ptr, item, this_p->item_size);
	++(this_p->len);
	return true;
}


// internal functions
static bool dlst_internal_integrity_chk(dlst_t const* this_p)
{
	if (this_p->ptr)
	{
		if (this_p->cap < this_p->len)
		{
			fprintf(stderr, "Unexpected error: Internal list error.\n");
			return false;
		}
	}

	return true;
}

static bool dlst_internal_realloc(dlst_t* this_p, dlst_len_t len)
{
	dlst_len_t cap;
	if (this_p->cap)
		cap = this_p->cap * 2;
	else
		cap = len;

	if (cap < len)
		cap = len;

	size_t new_size = cap * this_p->item_size;
	this_p->ptr = realloc(this_p->ptr, new_size);
	if (this_p->ptr == NULL)
	{
		fprintf(stderr, "No enough memory.\n");
		return false;
	}
	this_p->cap = cap;
	return true;
}
