// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "translator.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    char* ret = argv[1];

    dstr_t processedArg;

    while(ret != 0)
    {   
        dstr_init(&processedArg);
        dstr_assign_str(&processedArg, "");

        ret = translator(ret, &processedArg);
        printf("%s\n", processedArg.ptr);

        dstr_term(&processedArg);
    }

    return 0;
}