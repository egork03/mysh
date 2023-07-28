// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "base/bool.h"
#include "base/read.h"

#include "parser.h"
#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


bool run(char const* file, int* exit_code);

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;

    if (argc > 1)
    {
        for(int i = 1; i < argc; i++)
        {
            int ec;
            if (!run(argv[i], &ec))
            {
                exit_code = ec;
                break;
            }
        }
    }
    else
    {
        int ec;
        if (!run(0, &ec))
            exit_code = EXIT_FAILURE;
    }

    return exit_code;
}

bool run_interal_managed(read_input_state_t* input, int* exit_code)
{
    if (input->is_interactive)
        printf("Welcome to my shell!\n");

    bool result = true;
    while (1)
    {
        if(input->is_interactive)
        {
            if(result)
                printf("mysh> ");
            else
                printf("!mysh> ");

            fflush(stdout);
        }

        if (!read_input_get_line(input))
        {
            perror("getline");
            result = false;
            break;
        }

        if(input->is_eof)
            break;

        if(input->line.ptr[0] == '\n')
        {
            if (!input->is_interactive)
            {
                printf("\n");
            }
            continue;
        }

        command_node_t* cmd = parse_command_line(input->line.ptr);

        if (!input->is_interactive)
        {
            printf("mysh> %s", input->line.ptr);
        }

        if (cmd)
        {
            result = true;
        }
        else
        {
            result = false;

            if (!input->is_interactive)
            {
                *exit_code = -1;
                return false;
            }

            continue;
        }

        command_exec_status_t exec_status = { .code = 0, .exit = false };
        result = command_node_exec(cmd, &exec_status);
        command_node_term(cmd);

        //printf("\n");
        if (result)
        {
            if (exec_status.code != 0)
            {
                result = false;
                if (!input->is_interactive)
                {
                    *exit_code = exec_status.code;
                    return false;
                }
            }
        }
        else
        {
            if (!input->is_interactive)
            {
                *exit_code = -1;
                return false;
            }
        }
        
        if (exec_status.exit)
            break;
    }

    return result; 
}

bool run(char const* file, int* exit_code)
{
    read_input_state_t state;
    read_input_init(&state);

    if (!read_input_open(&state, file))
        return false;

    bool res = run_interal_managed(&state, exit_code);

    read_input_term(&state);
    return res;
}
