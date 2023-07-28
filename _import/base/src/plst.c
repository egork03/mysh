// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "plst.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if (_MSC_VER >= 1400)
#pragma warning(disable: 4996) // disabling deprecation for msvc
#endif

static bool plst_internal_integrity_chk(plst_t const* this_p);
static bool plst_internal_realloc(plst_t* this_p, plst_len_t len);

void plst_init(plst_t* this_p)
{ 
	this_p->ptr = 0; 
	this_p->len = 0; 
	this_p->cap = 0; 
}

void plst_term(plst_t* this_p, plst_item_term_func_t item_term_func)
{
	if (this_p->ptr != 0)
	{
		if (item_term_func)
		{
			for (plst_len_t i = 0; i < this_p->len; ++i)
			{
				item_term_func(this_p->ptr[i]);
			}
		}

		// protection against double free
		plst_item_t* p = this_p->ptr;
		this_p->ptr = 0;
		free(p);
	}
}

bool plst_is_null(plst_t const* this_p)
{ 
	return this_p->ptr == 0; 
}

bool plst_is_empty(plst_t const* this_p)
{
	return this_p->len == 0;
}

int plst_length(plst_t const* this_p)
{
	return this_p->len;
}

bool plst_append(plst_t* this_p, plst_item_t item)
{
	if (!plst_internal_integrity_chk(this_p))
		return false;

	plst_len_t avail = this_p->cap - 1;
	if (avail >= this_p->len)
	{
		this_p->ptr[this_p->len] = item;
		++(this_p->len);
		return true;
	}

 	plst_len_t new_len = this_p->len + 1;
	if (!plst_internal_realloc(this_p, new_len))
		return false;

	this_p->ptr[this_p->len] = item;
	++(this_p->len);
	return true;
}

bool plst_append_zero(plst_t* this_p)
{
	if (!plst_internal_integrity_chk(this_p))
		return false;

	plst_len_t avail = this_p->cap - 1;
	if (avail >= this_p->len)
	{
		this_p->ptr[this_p->len] = 0;
		return true;
	}

 	plst_len_t new_len = this_p->len + 1;
	if (!plst_internal_realloc(this_p, new_len))
		return false;

	this_p->ptr[this_p->len] = 0;
	return true;
}

// internal functions
static bool plst_internal_integrity_chk(plst_t const* this_p)
{
	if (this_p->ptr)
	{
		if (this_p->cap < this_p->len)
		{
			fprintf(stderr, "Unexpected error: Internal p-list error.\n");
			return false;
		}
	}

	return true;
}

static bool plst_internal_realloc(plst_t* this_p, plst_len_t len)
{
	plst_len_t cap;
	if (this_p->cap)
		cap = this_p->cap * 2;
	else
		cap = len;

	if (cap < len)
		cap = len;

	size_t new_size = cap * sizeof(plst_item_t);
	this_p->ptr = realloc(this_p->ptr, new_size);
	if (!this_p->ptr)
	{
		fprintf(stderr, "No enough memory.\n");
		return false;
	}
	this_p->cap = cap;
	return true;
}

bool plst_append_copy_from_view(plst_t* this_p, char const* str, int str_len)
{
	int l = (str_len + 1) * sizeof(char);
	char* d = malloc(l);
	if (!d)
	{
		fprintf(stderr, "No enough memory.\n");
		return false;
	}

	strcpy(d, str);
	
	// takes ownership of memory pointed by d (if succeeded)
	if (!plst_append(this_p, d))
	{
		free(d);
		return false;
	}

	return true;
}

bool plst_append_copy_from_str(plst_t* this_p, char const* str)
{
	int str_len = (int)strlen(str);
	return plst_append_copy_from_view(this_p, str, str_len);
}
