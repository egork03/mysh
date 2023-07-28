// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "base/dstr.h"
#include "base/bool.h"

// processes escapism before feeding to lexer
char* translator(char* input, dstr_t* arg)
{
    char* cur = input;
    char* next = cur + 1;

    while(*cur != 0 && *cur != 10 && *cur != 32)
    {
        if(*cur == '\\')
        {
            if(*next == '\\')
            {
                dstr_append_chr(arg, '\\');
                cur += 2;
                next += 2;
            }
            else if(*next == '<')
            {
                dstr_append_chr(arg, '<');
                cur += 2;
                next += 2;
            }
            else if(*next == '>')
            {
                dstr_append_chr(arg, '>');
                cur += 2;
                next += 2;
            }
            else if(*next == '|')
            {
                dstr_append_chr(arg, '|');
                cur += 2;
                next += 2;
            }
            else if(*next == ' ')
            {
                dstr_append_chr(arg, ' ');
                cur += 2;
                next += 2;
            }
        }
        else
        {
            dstr_append_chr(arg, *cur);
            cur++;
            next++;
        }        
    }

    if(*cur == 0)
    {
        dstr_append_chr(arg, *cur);
        return 0;
    }

    // get rid of intermediate whitespace
    do cur++;
    while(*cur == 32);

    return cur;
}