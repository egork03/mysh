// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "dstr.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool dstr_internal_integrity_chk(dstr_t const* this_p);
static bool dstr_internal_realloc(dstr_t* this_p, dstr_len_t len);
static bool dstr_internal_ensure_reserve(dstr_t* this_p, dstr_len_t cap);

void dstr_init(dstr_t* this_p) 
{ 
	this_p->ptr = 0; 
	this_p->len = 0; 
	this_p->cap = 0; 
}

void dstr_term(dstr_t* this_p)
{
	if (this_p->ptr != 0)
	{
		// protection against double free
		dstr_chr_t* p = this_p->ptr;
		this_p->ptr = 0;
		free(p);
	}
}

bool dstr_is_null(dstr_t const* this_p)
{ 
	return this_p->ptr == 0; 
}

bool dstr_is_empty(dstr_t const* this_p)
{
	return this_p->len == 0;
}

int dstr_length(dstr_t const* this_p)
{
	return this_p->len;
}

bool dstr_assign_view(dstr_t* this_p, dstr_chr_t const* ptr, dstr_len_t len)
{
	if (!dstr_internal_integrity_chk(this_p))
		return false;

	dstr_len_t c = len + 1;
	if (!dstr_internal_ensure_reserve(this_p, c))
		return false;

	memcpy(this_p->ptr, ptr, len * sizeof(dstr_chr_t));
	this_p->ptr[len] = 0;
	this_p->len = len;
	return true;
}

bool dstr_assign_str(dstr_t* this_p, dstr_chr_t const* ptr)
{
	dstr_len_t l = (dstr_len_t)strlen(ptr);
	return dstr_assign_view(this_p, ptr, l); 	
}

bool dstr_assign_dstr(dstr_t* this_p, dstr_t const* str)
{
	return dstr_assign_view(this_p, str->ptr, str->len); 	
}

bool dstr_append_chr(dstr_t* this_p, char c)
{
	if (!dstr_internal_integrity_chk(this_p))
		return false;

	dstr_len_t avail = this_p->cap - 1 - 1;
	if (avail >= this_p->len)
	{
		this_p->ptr[this_p->len] = c;
		++(this_p->len);
		this_p->ptr[this_p->len] = 0;
		return true;
	}

	dstr_len_t new_len = this_p->len + 1;
	if (!dstr_internal_realloc(this_p, new_len))
		return false;

	this_p->ptr[this_p->len] = c;
	++(this_p->len);
	this_p->ptr[this_p->len] = 0;
	return true;
}

bool dstr_append_view(dstr_t* this_p, dstr_chr_t const* ptr, dstr_len_t len)
{
	if (!dstr_internal_integrity_chk(this_p))
		return false;

	dstr_len_t avail = this_p->cap - this_p->len - 1;
	if (avail >= len)
	{
		dstr_chr_t* p = this_p->ptr + this_p->len;
		memcpy(p, ptr, len * sizeof(dstr_chr_t));
		p[len] = 0;
		this_p->len += len;
	}
	else
	{
		dstr_len_t new_len = this_p->len + len;
		if (!dstr_internal_realloc(this_p, new_len))
			return false;

		dstr_chr_t* p = this_p->ptr + this_p->len;
		memcpy(p, ptr, len * sizeof(dstr_chr_t));
		p[len] = 0;
		this_p->len += len;
	}

	return true;
}

bool dstr_append_str(dstr_t* this_p, dstr_chr_t const* ptr) 
{ 
	dstr_len_t l = (dstr_len_t)strlen(ptr);
	return dstr_append_view(this_p, ptr, l);
}

bool dstr_append_dstr(dstr_t* this_p, dstr_t const* str)
{
	return dstr_append_view(this_p, str->ptr, str->len);
}

// internal functions
static bool dstr_internal_integrity_chk(dstr_t const* this_p)
{
	if (this_p->ptr)
	{
		if (this_p->cap <= this_p->len)
		{
			fprintf(stderr, "Unexpected error: Internal string error.\n");
			return false;
		}
	}

	return true;
}

static bool dstr_internal_realloc(dstr_t* this_p, dstr_len_t len)
{
	dstr_len_t cap = this_p->cap;

	// dstr gross policy
	{
		if (cap)
			cap = cap * 2 + 1;

		if (cap <= len)
			cap = len + 1;
	}

	size_t new_size = cap * sizeof(dstr_chr_t);
	this_p->ptr = realloc(this_p->ptr, new_size);
	if (this_p->ptr == NULL)
	{
		fprintf(stderr, "No enough memory.\n");
		return false;
	}
	this_p->cap = cap;
	return true;
}

bool dstr_internal_ensure_reserve(dstr_t* this_p, dstr_len_t cap)
{
	if (this_p->cap >= cap)
		return true;

	if (this_p->ptr != 0)
		free(this_p->ptr);

	size_t size = cap * sizeof(dstr_chr_t);
	this_p->ptr = malloc(size);
	if (this_p->ptr == NULL)
	{
		fprintf(stderr, "Not enough memory.\n");
		return false;
	}

	this_p->cap = cap;
	return true;
}
