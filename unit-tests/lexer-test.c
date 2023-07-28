// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "lexer.h"
#include "token.h"
#include <stdio.h>

//char CMD1[] = "cd ..";
//char CMD2[] = "foo bar < baz | quux *.txt > spam";

int test(char const* cmd);

int main(int argc, char **argv)
{
    for(int i = 1; i < argc; i++)
    {
        char const* a = argv[i];
        printf("%s\n", a);
        printf("-----\n");
        if(!test(a))
            return 1;

        printf("\n");
    }

    return 0;
}

int test(char const* cmd)
{
    char const* cur = cmd;
    char const* next = cmd;

    token_t t;
    token_init(&t);

    while(1)
    {
        next = lexer(cur, &t);

        if(t.token_type == TOKEN_EOF)
        {
            break;
        }

        char const* ts = token_type_to_str(t.token_type);
        printf("'%s' - %s\n", t.token_text.ptr, ts);
        cur = next;
    }

    return 1;
}