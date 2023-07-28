// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

// read class for buffered IO

#pragma once

#include "bool.h"
#include "dstr.h"

#define read_input_buffer_MAX 512

typedef struct read_input_state_s
{
    int fin;
    bool is_interactive;
    bool is_eof;
    dstr_t line;
    int  buffer_bytes;
    int  buffer_pos;
    char buffer[read_input_buffer_MAX];
}
read_input_state_t;

// Construction
void read_input_init(read_input_state_t* this_p);
void read_input_term(read_input_state_t* this_p);

// Attrtibutes
bool     read_input_open(read_input_state_t* this_p, char const* file);
bool read_input_get_line(read_input_state_t* this_p);
