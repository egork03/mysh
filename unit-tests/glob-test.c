// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

// enable telldir(), seekdir() and DT_DIR when using glibc
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "base/dstr.h"
#include "base/plst.h"
#include "glob.h"

int main(int argc, char **argv)
{
    char* glob_path = (argc < 2)? "." : argv[1];

    plst_t files;
    plst_init(&files);

    int exit_code = EXIT_SUCCESS;
    plst_len_t added = 0;
    if (!glob_append(glob_path, &files, &added))
        exit_code = EXIT_FAILURE;

    for (plst_len_t i = 0; i < files.len; ++i)
	{
		char const* f = files.ptr[i];
		printf("%s\n", f);
	}

    plst_term(&files, (plst_item_term_func_t)free);
    return exit_code;
}
