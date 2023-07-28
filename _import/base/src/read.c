// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "read.h"
#include "dstr.h"
#include "bool.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__unix__) || defined(__CYGWIN__)
	#include <unistd.h>
#elif _WIN32
	#include <io.h>
#endif

#include <string.h>
#include <fcntl.h>
#include <assert.h>


#if (_MSC_VER >= 1400)
#pragma warning(disable: 4996) // disabling deprecation for msvc
#endif

void read_input_init(read_input_state_t* thip_p)
{
    thip_p->fin = 0;
    thip_p->is_interactive = false;
	thip_p->is_eof = false;
	dstr_init(&thip_p->line);
    thip_p->buffer_pos = 0;
    thip_p->buffer_bytes = 0;
}

void read_input_term(read_input_state_t* thip_p)
{
   if (thip_p->fin)
   {
		int fin = thip_p->fin;
		thip_p->fin = 0;
        close(fin);
   }

   dstr_term(&thip_p->line);
}

bool read_input_open(read_input_state_t* this_p, char const* file)
{
    if (file)
    {
        this_p->fin = open(file, O_RDONLY);
        if (this_p->fin == -1)
        {
            perror(file);
            return false;
        }
        this_p->is_interactive = false;
    }
    else
    {
        this_p->fin = 0;
        this_p->is_interactive = true;
    }

	return true;
}

bool read_input_get_line(read_input_state_t* this_p)
{
	this_p->line.len = 0;

	while (true)
	{
		int pos;
		for (pos = this_p->buffer_pos; pos < this_p->buffer_bytes; ++pos)
		{
			if (this_p->buffer[pos] == '\n')
			{
				int l = pos - this_p->buffer_pos + 1;
				if(!dstr_append_view(&this_p->line, this_p->buffer + this_p->buffer_pos, l))
					return false;

				this_p->buffer_pos = pos + 1;
				return true;
			}
		}

		int l = pos - this_p->buffer_pos;
		if (l)
		{
			if(!dstr_append_view(&this_p->line, this_p->buffer + this_p->buffer_pos, l))
				return false;
		}

		this_p->buffer_pos = 0;

		// read input
		if (!this_p->is_eof)
		{
			this_p->buffer_bytes = read(this_p->fin, this_p->buffer, read_input_buffer_MAX);
			if (this_p->buffer_bytes < 0)
			{
				perror("read");
				return false;
			}
		}

		//handle EOF
		if (this_p->buffer_bytes == 0)
		{
			this_p->is_eof = true;
			return true;
		}
	}

    return true;
}
