// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "glob.h"
#include "base/dstr.h"

#if defined(__unix__) || defined(__CYGWIN__)
	// enable telldir(), seekdir() and DT_DIR when using glibc
	#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <string.h>

#if defined(__unix__) || defined(__CYGWIN__)
	#include <dirent.h>
#elif _WIN32
	#include <io.h>
#endif

static int glob(char const* pattern, char const* pattern_end, char const* str);
static bool collect_glob_internal_managed(char const* dname, char const* pattern, plst_t* files);
static char const* get_next_pattern(char const* pattern, char const** pattern_end, bool* is_pattern_subdir);

bool glob_append(char const* glob_path, plst_t* files, plst_len_t* files_added)
{
    int chr_idx = strcspn(glob_path, "*"); // "*?"
    char const* chr_ptr = glob_path + chr_idx;

    if (!*chr_ptr)
    {
        *files_added = 0;
        return true;
    }

    char const* pattern;
    dstr_t dname;
    dstr_init(&dname);

    // '/usr/ab*c/bcd/2.txt'
    //         ^
    char const* chr_dir = chr_ptr;
    while (true)
    {
        --chr_dir;
        if (chr_dir < glob_path)
        {
            chr_dir = 0;
            break;
        }

        if (*chr_dir == '/')
            break;
    }

    if (chr_dir && (chr_dir < chr_ptr))
    {
        // '/usr/ab*c/bcd/2.txt'
        //      ^
        //   dname: '/usr'
        // pattern: 'ab*c/bcd/2.txt'
        int l = chr_dir - glob_path;
        if (!dstr_append_view(&dname, glob_path, l))
        {
            dstr_term(&dname);
            return false;
        }
        pattern = chr_dir + 1;
    }
    else
    {
        // 'usr*/abc/bcd/2.txt'
        // ^
        //   dname: ''
        // pattern: 'usr*/abc/bcd/2.txt'
        if (!dstr_append_chr(&dname, '.'))
        {
            dstr_term(&dname);
            return false;
        }
        pattern = glob_path;
    }

    plst_len_t old_len = files->len;
    bool result = collect_glob_internal_managed(dname.ptr, pattern, files);
    plst_len_t new_len = files->len;

    if (files_added)
        *files_added = new_len - old_len;
    
    dstr_term(&dname);
    return result;
}

int glob(char const* pattern, char const* pattern_end, char const* str)
{
    const char * here;

    while (true)
    {
        char c = *pattern;

        ++pattern;
        if (pattern > pattern_end)
            return *str ? -1 : 0;

        switch (c)
        {
        case '\0':
            return *str ? -1 : 0;

        //case '?':
        //    if (!*str++)
        //        return 1;
        //    break;

        case '*':
            here = str;

            while (*str)
                ++str;

            while ( str != here )
            {
                int r;

                if (*pattern)
                    r = glob(pattern, pattern_end, str);
                else
                    r = (*str) ? -1 : 0;

                if ( !r )
                    return 0;
                if ( r < 0 )
                    return 1;
                --str;
            }
            break;

        default:
            if ( *str++ != pattern[-1] )
                return 1;
            break;
        }
    }
}

char const* get_next_pattern(char const* pattern, char const** pattern_end, bool* is_pattern_subdir)
{
    int chr_idx = strcspn(pattern, "*/"); // "*?/"
    char const* chr_ptr = pattern + chr_idx;
    
    // 'abc/bcd/*/*.txt'
    //     ^
    if (*chr_ptr == '/')
    {
        *pattern_end = chr_ptr;
        *is_pattern_subdir = true;
        
        // return: 'abc'
        //   next: 'bcd/*/*.txt'
        return chr_ptr + 1;
    }

    if (*chr_ptr != 0)
    {
        // 'ab*c/bcd/2.txt'
        //    ^
        char const* chr_dir = strchr(chr_ptr, '/');
        if (chr_dir)
        {
            *pattern_end = chr_dir;
            *is_pattern_subdir = true;

            // return: 'ab*c'
            //   next: 'bcd/2.txt'
            return chr_dir + 1;
        }
    }

    // '*.txt' OR 'abc.txt'
    char const* end = pattern + strlen(pattern);
    *pattern_end = end;
    *is_pattern_subdir = false;

    // return: '*.txt' OR 'abc.txt'
    //   next: ''
    return end;
}

bool collect_glob_internal(char const* dname, DIR* dp, char const* pattern, plst_t* files, dstr_t* inner_dir_memory_holder, dstr_t* inner_file_memory_holder)
{
    char const* pattern_end;
    bool is_pattern_subdir;
    char const* next_pattern = get_next_pattern(pattern, &pattern_end, &is_pattern_subdir);

    struct dirent* de;
    while ((de = readdir(dp)))
    {
        if 
        (
            is_pattern_subdir == false 
                && de->d_type == DT_REG 
                && de->d_name[0] != '.'
        )
        {
            if ((pattern >= pattern_end) || (glob(pattern, pattern_end, de->d_name) == 0))
            {
                if (!dstr_assign_str(inner_file_memory_holder, dname))
                    return false;

                if (!dstr_append_chr(inner_file_memory_holder, '/'))
                    return false;

                if (!dstr_append_str(inner_file_memory_holder, de->d_name))
                    return false;

                if (!plst_append_copy_from_view(files, inner_file_memory_holder->ptr, inner_file_memory_holder->len))
                    return false;
            }
        }
        else if 
        (
            is_pattern_subdir == true 
                && de->d_type == DT_DIR 
                && de->d_name[0] != '.'
        ) 
        {
            if ((pattern >= pattern_end) || (glob(pattern, pattern_end, de->d_name) == 0))
            {
                // form new path
                if (!dstr_assign_str(inner_dir_memory_holder, dname))
                    return false;

                if (!dstr_append_chr(inner_dir_memory_holder, '/'))
                    return false;

                if (!dstr_append_str(inner_dir_memory_holder, de->d_name))
                    return false;

                // save location in directory
                long offset = telldir(dp);
                if (offset == -1)
                {
                    perror(dname);
                    return false;
                }
                closedir(dp);

                // recursively traverse subdirectory
                if (!collect_glob_internal_managed(inner_dir_memory_holder->ptr, next_pattern, files))
                    return false;

                // restore position in directory
                dp = opendir(dname);
                if (!dp)
                {
                    perror(dname);
                    return false; 
                }
                seekdir(dp, offset);
            }
        }
    }

    return true;
}

static bool collect_glob_internal_managed(char const* dname, char const* pattern, plst_t* files)
{
    DIR* dp = opendir(dname);
    if (!dp) 
    {
        perror(dname);
        return false;
    }

    dstr_t dstr_dir, dstr_file;
    dstr_init(&dstr_dir);
    dstr_init(&dstr_file);

    bool result = collect_glob_internal(dname, dp, pattern, files, &dstr_dir, &dstr_file);

    dstr_term(&dstr_dir);
    dstr_term(&dstr_file);
    
    closedir(dp);
    
    return result;
}
