// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void command_node_type_check_fail(command_combine_type_t command_combine_type);
void command_init(command_t* this_p);
void command_term(command_t* this_p);

static char* command_arg_compose(dstr_t const* token_text)
{
    char* p = malloc((token_text->len + 1) * sizeof(char));
    if (!p)
    {
		fprintf(stderr, "No enough memory.\n");
        return 0;
    }

    memcpy(p, token_text->ptr, token_text->len);
    p[token_text->len] = 0;

    return p;
}

static command_node_t* command_node_compose_single(dlst_t* pileline)
{
    command_node_t* p = malloc(sizeof(command_node_t));
    if (!p)
    {
		fprintf(stderr, "No enough memory.\n");
        return 0;
    }

    p->combine_type = COMMAND_COMBINE_PIPE;
    p->pileline = *pileline;
    return p;
}

static command_node_t* command_node_compose_binary(command_combine_type_t combine_type, command_node_t* node_left , dlst_t* pileline_right)
{
    if (combine_type != COMMAND_COMBINE_AND && combine_type != COMMAND_COMBINE_OR)
    {
        command_node_type_check_fail(combine_type);
        return 0;
    }

    command_node_t* p = malloc(sizeof(command_node_t));
    if (!p)
    {
		fprintf(stderr, "No enough memory.\n");
        return 0;
    }

    command_node_t* right = command_node_compose_single(pileline_right);
    if (!right)
        return false;

    p->combine_type = combine_type;
    p->left = node_left;
    p->right = right;
    return p;
}
