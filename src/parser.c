// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "parser.h"
#include "token.h"  // includes 'parser.token.h'
#include "parser.error.h"
//#include "parser.state.inl"
#include "lexer.h"
#include "base/bool.h"
#include "command.h"

#include <stdio.h>
#include <stdlib.h>

#include "parser.inl"

#undef EOF 
#define EOF 0


static inline void get(parser_t* this_p);
static inline bool expect(parser_t* this_p, token_type_t tt);
static void        syntax_error_state(parser_t* this_p, error_type_t et);
static void        syntax_error_token_expected(parser_t* this_p, token_type_t tt);
//static inline bool start_of(parser_t* this_p, parser_state_t pt);

static inline void get(parser_t* this_p)
{
	token_t* const p = this_p->t;
	this_p->t = this_p->la;
	this_p->la = p;
	this_p->pos = lexer(this_p->pos, this_p->la);
}

static inline bool expect(parser_t* this_p, token_type_t tt)
{
	if (this_p->la->token_type==tt) 
	{
		get(this_p); 
	}
	else 
	{ 
		syntax_error_token_expected(this_p, tt);
		return false; 
	}
	return true;
}

static bool parser(parser_t* this_p);
static bool command_pipeline(parser_t* this_p, dlst_t* pipeline);
static bool redirected_command(parser_t* this_p, command_t* cmd);
static bool command(parser_t* this_p, command_t* cmd);
static bool command_args(parser_t* this_p, plst_t* args);
static bool command_redir_list(parser_t* this_p, dstr_t* redir_in_from, dstr_t* redir_out_to);
static bool command_redir(parser_t* this_p, dstr_t* redir_in_from, dstr_t* redir_out_to);


static bool parser(parser_t* this_p)
{
	command_node_t* n;
	dlst_t pipeline;
	dlst_init(&pipeline, sizeof(command_t));
	if (!command_pipeline(this_p, &pipeline))
		return false;
	n = command_node_compose_single(&pipeline);
	if (!n)
	return false;
	this_p->cmd_root_node = n;
	while (this_p->la->token_type == TOKEN_AND || this_p->la->token_type == TOKEN_OR)
	{
		command_combine_type_t command_combine_type = COMMAND_COMBINE_NONE;
		if (this_p->la->token_type == TOKEN_OR)
		{
			get(this_p);
			command_combine_type = COMMAND_COMBINE_OR;
		}
		else
		{
			get(this_p);
			command_combine_type = COMMAND_COMBINE_AND;
		}
		dlst_init(&pipeline, sizeof(command_t));
		if (!command_pipeline(this_p, &pipeline))
			return false;
		n = command_node_compose_binary(command_combine_type, this_p->cmd_root_node, &pipeline);
		if (!n)
		return false;
		this_p->cmd_root_node = n;
	}

	return true;
}

static bool command_pipeline(parser_t* this_p, dlst_t* pipeline)
{
	command_t cmd;
	command_init(&cmd);
	if (!redirected_command(this_p, &cmd))
		return false;
	if (!dlst_append(pipeline, &cmd))
	return false;
	while (this_p->la->token_type == TOKEN_PIPE)
	{
		get(this_p);
		command_init(&cmd);
		if (!redirected_command(this_p, &cmd))
			return false;
		if (!dlst_append(pipeline, &cmd))
		return false;
	}

	return true;
}

static bool redirected_command(parser_t* this_p, command_t* cmd)
{
	if (!command(this_p, cmd))
		return false;
	if (this_p->la->token_type == TOKEN_PATH)
	{
		if (!command_args(this_p, &cmd->args))
			return false;
	}
	if (this_p->la->token_type == TOKEN_REDIRECTION_IN || this_p->la->token_type == TOKEN_REDIRECTION_OUT)
	{
		if (!command_redir_list(this_p, &cmd->redir_in_from, &cmd->redir_out_to))
			return false;
	}

	return true;
}

static bool command(parser_t* this_p, command_t* cmd)
{
	if (this_p->la->token_type == TOKEN_PATH)
	{
		get(this_p);
		cmd->command_type = COMMAND_EXTERNAL;
		if (!dstr_assign_dstr(&cmd->executable, &this_p->t->token_text))
		return false;

		char* arg0 = command_arg_compose(&this_p->t->token_text);
		if (!arg0)
		return false;
		if (!plst_append(&cmd->args, arg0))
		return false;
	}
	else if (this_p->la->token_type == TOKEN_COMMAND_CD)
	{
		get(this_p);
		cmd->command_type = COMMAND_BUILTIN_CD;
		if (!dstr_assign_dstr(&cmd->executable, &this_p->t->token_text))
		return false;
	}
	else if (this_p->la->token_type == TOKEN_COMMAND_PWD)
	{
		get(this_p);
		cmd->command_type = COMMAND_BUILTIN_PWD;
		if (!dstr_assign_dstr(&cmd->executable, &this_p->t->token_text))
		return false;
	}
	else if (this_p->la->token_type == TOKEN_COMMAND_EXIT)
	{
		get(this_p);
		cmd->command_type = COMMAND_BUILTIN_EXIT;
		if (!dstr_assign_dstr(&cmd->executable, &this_p->t->token_text))
		return false;
	}
	else
	{
		syntax_error_state(this_p, SYNTAX_ERROR_invalid_command);
		return false;
	}

	return true;
}

static bool command_args(parser_t* this_p, plst_t* args)
{
	char* p;
	if (!expect(this_p, TOKEN_PATH))
		return false;
	p = command_arg_compose(&this_p->t->token_text);
	if (!p)
	return false;

	if (!plst_append(args, p))
	return false;
	while (this_p->la->token_type == TOKEN_PATH)
	{
		get(this_p);
		p = command_arg_compose(&this_p->t->token_text);
		if (!p)
		return false;

		if (!plst_append(args, p))
		return false;
	}
	if (!plst_append_zero(args))
	return false;

	return true;
}

static bool command_redir_list(parser_t* this_p, dstr_t* redir_in_from, dstr_t* redir_out_to)
{
	if (!command_redir(this_p, redir_in_from, redir_out_to))
		return false;
	while (this_p->la->token_type == TOKEN_REDIRECTION_IN || this_p->la->token_type == TOKEN_REDIRECTION_OUT)
	{
		if (!command_redir(this_p, redir_in_from, redir_out_to))
			return false;
	}

	return true;
}

static bool command_redir(parser_t* this_p, dstr_t* redir_in_from, dstr_t* redir_out_to)
{
	if (this_p->la->token_type == TOKEN_REDIRECTION_IN)
	{
		get(this_p);
		if (!expect(this_p, TOKEN_PATH))
			return false;
		if (!dstr_is_null(redir_in_from))
		{
		fprintf(stderr, "error: excessive in-redirection '%s'\n", this_p->t->token_text.ptr);
		return false;
		}
		if (!dstr_assign_dstr(redir_in_from, &this_p->t->token_text))
		return false;
	}
	else if (this_p->la->token_type == TOKEN_REDIRECTION_OUT)
	{
		get(this_p);
		if (!expect(this_p, TOKEN_PATH))
			return false;
		if (!dstr_is_null(redir_out_to))
		{
		fprintf(stderr, "error: excessive out-redirection '%s'\n", this_p->t->token_text.ptr);
		return false;
		}
		if (!dstr_assign_dstr(redir_out_to, &this_p->t->token_text))
		return false;
	}
	else
	{
		syntax_error_state(this_p, SYNTAX_ERROR_invalid_command_redir);
		return false;
	}

	return true;
}



command_node_t* parse_command_line(char const* command_line)
{
	parser_t p;
	parser_t* this_p = &p;

	token_t tokens[2];
	token_init(&tokens[0]);
	token_init(&tokens[1]);
	this_p->pos = this_p->str = command_line;
	this_p->t = tokens + 0;
	this_p->la = tokens + 1;
	this_p->cmd_root_node = 0;

	get(this_p);

	bool result;
	while (true)
	{
		if (!(result = parser(this_p)))
			break;
		if (!(result = expect(this_p, EOF)))
			break;

		break;
	}

	token_term(&tokens[0]);
	token_term(&tokens[1]);

	if (!result)
	{
		if (this_p->cmd_root_node)
		{
			command_node_term(this_p->cmd_root_node);
			this_p->cmd_root_node = 0;
		}
		return 0;
	}
	
	return this_p->cmd_root_node;
}

static void syntax_error_state(parser_t* this_p, error_type_t et)
{
	printf("Syntax error: %d\n", et);
}

static void syntax_error_token_expected(parser_t* this_p, token_type_t tt)
{
	printf("Error: expected token %d\n", tt);
}

