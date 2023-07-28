// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once

#include "base/dstr.h"
#include "base/dlst.h"
#include "base/plst.h"
#include "base/bool.h"

typedef enum command_type_e
{
	COMMAND_NONE = 0,
	COMMAND_EXTERNAL,
	COMMAND_BUILTIN_CD,
	COMMAND_BUILTIN_PWD,
	COMMAND_BUILTIN_EXIT
}
command_type_t;


typedef struct command_s
{
	// original input data
	command_type_t command_type;
	dstr_t executable;
	plst_t args;
	dstr_t redir_in_from;
	dstr_t redir_out_to;

	// operational data
	dstr_t executable_path_resolved;
	plst_t args_glob_refined;
	int pid;
	int pipe_in;
	int pipe_out;
	int exit_code;
}
command_t;

typedef enum command_combine_type_e
{
	COMMAND_COMBINE_NONE = 0,
	COMMAND_COMBINE_PIPE,
	COMMAND_COMBINE_AND,
	COMMAND_COMBINE_OR,
}
command_combine_type_t;

typedef struct command_node_s command_node_t;

typedef struct command_node_s
{
	command_combine_type_t combine_type;

	union
	{
		struct // valid if combine_type == AND || combine_type == OR
		{
			command_node_t* left;
			command_node_t* right;
		};

		struct // valid if combine_type == PIPE
		{
			union
			{
				dlst_t pileline; 
				struct // for debug viewing
				{
					command_t* dbg_pileline_ptr;
					dlst_len_t dbg_pileline_cap;
					dlst_len_t dbg_pileline_len;
				};
			};
		};
	};
}
command_node_t;

typedef struct command_exec_status_s
{
	int code;
	bool exit; // to support interactive mode exit
	int wait_count;
}
command_exec_status_t;

void command_exec_external_echo(char const* prefix, command_t const* c);

bool command_node_exec(command_node_t* this_p, command_exec_status_t* exec_status);
void command_node_term(command_node_t* this_p);
