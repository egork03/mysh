// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "command.h"
#include "base/dstr.h"
#include "base/dlst.h"
#include "glob.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#if defined(__unix__) || defined(__CYGWIN__)
	#include <unistd.h>
	#include <sys/wait.h>
#elif _WIN32
	#include <io.h>
#endif

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#if (_MSC_VER >= 1400)
#pragma warning(disable: 4996) // disabling deprecation for msvc
#endif

void command_init(command_t* this_p)
{
    this_p->command_type = COMMAND_NONE;
    dstr_init(&(this_p->executable));
    plst_init(&(this_p->args));
    dstr_init(&(this_p->redir_in_from));
    dstr_init(&(this_p->redir_out_to));

    dstr_init(&(this_p->executable_path_resolved));
    plst_init(&(this_p->args_glob_refined));
    this_p->pid = 0;
    this_p->pipe_in = 0;
    this_p->pipe_out = 0;
    this_p->exit_code = 0;
}

void command_term(command_t* this_p)
{
    dstr_term(&(this_p->executable));
    plst_term(&(this_p->args), (plst_item_term_func_t)free);
    dstr_term(&(this_p->redir_in_from));
    dstr_term(&(this_p->redir_out_to));

    dstr_term(&(this_p->executable_path_resolved));
    plst_term(&(this_p->args_glob_refined), (plst_item_term_func_t)free);
}

static char const* command_get_executable(command_t const* c)
{
    char const* executable = c->executable_path_resolved.ptr;
    if (executable)
        return executable;

    return c->executable.ptr;
}

static bool command_exec_sys_error_msg(command_t const* c, char const* str_error)
{
    if (c)
    {
        char const* executable = command_get_executable(c);
        fprintf(stderr, "error: %s: %s\n", executable, str_error);
        return true;
    }

    fprintf(stderr, "error: %s\n", str_error);
    return true;
}

static bool command_exec_builtin_cd  (command_t const* c, command_exec_status_t* exec_status);
static bool command_exec_builtin_pwd (command_t const* c, command_exec_status_t* exec_status);
static bool command_exec_builtin_exit(command_t const* c, command_exec_status_t* exec_status);
static bool command_exec_external    (command_t* c, command_exec_status_t* exec_status);

static bool command_exec(command_t* c, command_exec_status_t* exec_status)
{
    switch(c->command_type)
    {
        case COMMAND_BUILTIN_CD:
            return command_exec_builtin_cd(c, exec_status);
        case COMMAND_BUILTIN_PWD:
            return command_exec_builtin_pwd(c, exec_status);
        case COMMAND_BUILTIN_EXIT:
            return command_exec_builtin_exit(c, exec_status);
        case COMMAND_EXTERNAL:
            return command_exec_external(c, exec_status);
        default:
            break;
    }

    fprintf(stderr, "Unexpected error: invalid command type '%d'\n", c->command_type);
    exec_status->code = 1;
    return false;
}

static bool command_exec_builtin_cd_internal(command_t const* c, char const* path, command_exec_status_t* exec_status)
{
    int res = chdir(path);
    if(res == -1)
    {
    	exec_status->code = errno;
        char const* str_error = strerror(errno);
        command_exec_sys_error_msg(c, str_error);
        return false;
    }

    exec_status->code = 0;
    return true;
}

static bool command_exec_builtin_cd_home(command_t const* c, command_exec_status_t* exec_status)
{
    char* home_path = getenv("HOME");

    if(!home_path)
    {
        command_exec_sys_error_msg(c, "enviroment variable 'HOME' is not set");
        exec_status->code = -1;
        return false;
    }

    return command_exec_builtin_cd_internal(c, home_path, exec_status);
}

static bool command_exec_builtin_cd(command_t const* c, command_exec_status_t* exec_status)
{
    if(plst_is_null(&c->args))
        return command_exec_builtin_cd_home(c, exec_status);

    if (plst_length(&c->args) > 1)
    {
    	exec_status->code = -1;
        command_exec_sys_error_msg(c, "too many arguments");
        return false;
    }

    return command_exec_builtin_cd_internal(c, (char const*)(c->args.ptr[0]), exec_status);
}

static bool command_exec_builtin_pwd(command_t const* c, command_exec_status_t* exec_status)
{
    if (plst_length(&c->args) > 0)
    {
    	exec_status->code = -1;
        command_exec_sys_error_msg(c, "too many arguments");
        return false;
    }

    char* cwd = getcwd(NULL, 0);
    if(cwd == NULL)
    {
    	exec_status->code = errno;
        char const* str_error = strerror(errno);
        command_exec_sys_error_msg(c, str_error);
        return false;
    }

    printf("%s\n", cwd);
    free(cwd);

    exec_status->code = 0;
    return true;
}

static bool command_exec_builtin_exit(command_t const* c, command_exec_status_t* exec_status)
{
    if (plst_length(&c->args) > 1)
    {
    	exec_status->code = -1;
        command_exec_sys_error_msg(c, "too many arguments");
        return false;
    }

    if(plst_is_null(&c->args))
    {
        exec_status->code = 0;
        exec_status->exit = true;
        return false;
    }
    
    char* endptr = 0;
    int ret = strtol((char const*)c->args.ptr[0], &endptr, 10);

    if(endptr == 0 || *endptr != 0)
    {
    	exec_status->code = -1;
        command_exec_sys_error_msg(c, "numeric argument required");
        return false;        
    }

    exec_status->code = ret;
    exec_status->exit = true;
    return true;
}

static bool command_exec_external_check_prefix(char const* prefix, char const* cmd, dstr_t* cmd_resolved)
{
    dstr_t path;
    dstr_init(&path);
    if(prefix)
        dstr_assign_str(&path, prefix);
    dstr_append_str(&path, cmd);

    struct stat stat_struct;
    if(stat(path.ptr, &stat_struct) != 0)
    {
        dstr_term(&path);
        return false;
    }

    *cmd_resolved = path;
    return true;
}

static bool command_exec_external_search(char const* cmd, dstr_t* cmd_resolved)
{
    if (strchr(cmd, '/'))
    {
        return false;
    }

    if(command_exec_external_check_prefix("/usr/local/sbin/", cmd, cmd_resolved))
        return true;
    
    if(command_exec_external_check_prefix("/usr/local/bin/", cmd, cmd_resolved))
        return true;

    if(command_exec_external_check_prefix("/usr/sbin/", cmd, cmd_resolved))
        return true;

    if(command_exec_external_check_prefix("/usr/bin/", cmd, cmd_resolved))
        return true;        

    if(command_exec_external_check_prefix("/sbin/", cmd, cmd_resolved))
        return true;

    if(command_exec_external_check_prefix("/bin/", cmd, cmd_resolved))
        return true;

    return false;
}

static bool command_exec_external(command_t* c, command_exec_status_t* exec_status)
{
    plst_len_t argc = plst_length(&c->args);
    if (argc < 1)
    {
        exec_status->code = -1;
        fprintf(stderr, "Unexpected error: invalid number of arguments '%d'\n", argc);
        return false;
    }

    if (!plst_append_copy_from_str(&c->args_glob_refined, c->args.ptr[0]))
        return false;
    
    // refine args with wildcard expansion
    for (plst_len_t i = 1; i < c->args.len; ++i)
	{
		char const* f = c->args.ptr[i];
        plst_len_t added = 0;
        if (!glob_append(f, &c->args_glob_refined, &added))
            return false;

        if (!added)
        {
            if (!plst_append_copy_from_str(&c->args_glob_refined, f))
                return false;
        }
	}

    if (!plst_append_zero(&c->args_glob_refined))
        return false;

    if(!command_exec_external_check_prefix(0, c->executable.ptr, &c->executable_path_resolved))
    {
        if(!command_exec_external_search(c->executable.ptr, &c->executable_path_resolved))
        {
            exec_status->code = -1;
            command_exec_sys_error_msg(c, "No such external command");
            return false;
        }
    }

	int pid = fork();
    if (pid == -1)
    {
        exec_status->code = errno;
        char const* str_error = strerror(errno);
        command_exec_sys_error_msg(c, str_error);
        return false;
    }

    c->pid = pid;
	if (c->pid == 0)
    {
        int child_exit_code = 0;

        if (!dstr_is_null(&c->redir_out_to))
        {
            int fout = open(c->redir_out_to.ptr, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP);
            if(fout == -1)
            {
                child_exit_code = errno;
                char const* str_error = strerror(errno);
                command_exec_sys_error_msg(c, str_error);
                exit(child_exit_code ? child_exit_code : EXIT_FAILURE);
                return false;
            }

            dup2(fout, STDOUT_FILENO);
            close(fout);
        }
        else if (c->pipe_out)
        {
            dup2(c->pipe_out, STDOUT_FILENO);
            close(c->pipe_out);
        }

        if (!dstr_is_null(&c->redir_in_from))
        {
            int fin = open(c->redir_in_from.ptr, O_RDONLY);
            if(fin == -1)
            {
                child_exit_code = errno;
                char const* str_error = strerror(errno);
                command_exec_sys_error_msg(c, str_error);
                exit(child_exit_code ? child_exit_code : EXIT_FAILURE);
                return false;
            }

            dup2(fin, STDIN_FILENO);
            close(fin);
        }
        else if (c->pipe_in)
        {
            dup2(c->pipe_in, STDIN_FILENO);
            close(c->pipe_in);
        }

		// must use execv, because the number of arguments is dynamic
		execvp(c->executable_path_resolved.ptr, (char * const*)c->args_glob_refined.ptr);
       	
        child_exit_code = errno;
        if (!child_exit_code)
            child_exit_code = EXIT_FAILURE;

        command_exec_external_echo("execvp", c);
        printf(" : abnormally exited with code %d\n", child_exit_code);
        exit(child_exit_code);
        return false; // unreacheable code
	}

    ++(exec_status->wait_count);
    return true;
}

bool command_pileline_exec(dlst_t* command_pipeline, command_exec_status_t* exec_status)
{
    exec_status->wait_count = 0;

    if (dlst_length(command_pipeline) == 1)
    {
        command_t* cmd = dlst_at(command_pipeline, 0);
        if (!command_exec(cmd, exec_status))
			return false;

		wait(&exec_status->code);
		return true;
    }

    int prev_pipe_out = 0;
	for (dlst_len_t i = 0, end = command_pipeline->len - 1; i < end; ++i)
	{
		command_t* cmd = dlst_at(command_pipeline, i); 

        int p[2];
        if (pipe(p) == -1)
        {
            exec_status->code = errno;
            char const* str_error = strerror(errno);
            command_exec_sys_error_msg(0, str_error);
            return false;
        }

        cmd->pipe_out = p[1];
        cmd->pipe_in = prev_pipe_out;
        prev_pipe_out = p[0];

		if (!command_exec(cmd, exec_status))
            return false;

        if (cmd->pipe_in)
            close(cmd->pipe_in);
        if (cmd->pipe_out)
            close(cmd->pipe_out);
	}

    command_t* last_cmd = dlst_at(command_pipeline, command_pipeline->len - 1);
    last_cmd->pipe_in = prev_pipe_out;
    last_cmd->pipe_out = 0;
    if (!command_exec(last_cmd, exec_status))
        return false;

    if (last_cmd->pipe_in)
        close(last_cmd->pipe_in);
    if (last_cmd->pipe_out)
        close(last_cmd->pipe_out);

    for (dlst_len_t i = 0; i < exec_status->wait_count; ++i)
	{
        int exit_code = 0;
        int pid = wait(&exit_code);
        if (pid <= 0)
            break;

        for (dlst_len_t j = 0; j < command_pipeline->len; ++j)
        {
            command_t* cmd = dlst_at(command_pipeline, j);
            if (cmd->pid == pid)
            {
                cmd->exit_code = exit_code;
                break;
            }
        }
    }

    exec_status->code = last_cmd->exit_code;
    return true;
}

void command_node_type_check_fail(command_combine_type_t command_combine_type)
{
    fprintf(stderr, "Unexpected error: invalid combine type '%d'\n", command_combine_type);
}

bool command_node_exec(command_node_t* this_p, command_exec_status_t* exec_status)
{
    switch (this_p->combine_type)
    {
        case COMMAND_COMBINE_PIPE:
            return command_pileline_exec(&this_p->pileline, exec_status);
        
        case COMMAND_COMBINE_AND:
        {
            if (!command_node_exec(this_p->left, exec_status))
                return false;

            if (exec_status->code != 0)
                return true;

            return command_node_exec(this_p->right, exec_status);
        }
        case COMMAND_COMBINE_OR:
        {
            if (!command_node_exec(this_p->left, exec_status))
                return false;

            if (exec_status->code != 0)
                return command_node_exec(this_p->right, exec_status);

            return true;
        }
        default:
            break;
    }

    command_node_type_check_fail(this_p->combine_type);
    exec_status->code = 1;
    return false;
}

void command_term(command_t* this_p);

void command_node_term(command_node_t* this_p)
{
    switch (this_p->combine_type)
    {
        case COMMAND_COMBINE_PIPE:
        {
            dlst_term(&this_p->pileline, (dlst_item_term_func_t)command_term);
            break;
        }
        
        case COMMAND_COMBINE_AND:
        case COMMAND_COMBINE_OR:
        {
            command_node_term(this_p->left);
            command_node_term(this_p->right);
            break;
        }
        default:
            command_node_type_check_fail(this_p->combine_type);
            break;
    }

    free(this_p);
}

void command_exec_external_echo(char const* prefix, command_t const* c)
{
    char const* executable = command_get_executable(c);
    printf("%s: '%s'", prefix, executable);

    if (!plst_is_null(&c->args))
    {
        for (plst_len_t i = 1; i < c->args_glob_refined.len; ++i)
        {
            printf(" '%s'", (char const*)c->args_glob_refined.ptr[i]);
        }
    }

    if (!dstr_is_null(&c->redir_in_from))
    {
        printf(" < '%s'", c->redir_in_from.ptr);
    }

    if (!dstr_is_null(&c->redir_out_to))
    {
        printf(" > '%s'", c->redir_out_to.ptr);
    }
}
