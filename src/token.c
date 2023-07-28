// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#include "token.h"

char const* token_type_to_str(token_type_t tt)
{
    switch(tt)
    {
        case TOKEN_EOF:
            return "<end of file>";
	    case TOKEN_ERROR:
            return "<illegal character>";
        case TOKEN_COMMAND_CD:
            return "COMMAND_CD";
        case TOKEN_COMMAND_PWD:
            return "COMMAND_PWD";
        case TOKEN_PATH:
            return "PATH";        
        case TOKEN_WILDCARD:
            return "WILDCARD";
        case TOKEN_REDIRECTION_IN:
            return "REDIRECTION_IN";
        case TOKEN_REDIRECTION_OUT:
            return "REDIRECTION_OUT";
        case TOKEN_PIPE:
            return "PIPE";
        case TOKEN_AND:
            return "AND";
        case TOKEN_OR:
            return "OR";
        default:
            return "<unexpected token type>";
    }    
}

void token_init(token_t* t)
{
    dstr_init(&(t->token_text));
    t->token_type = TOKEN_ERROR;
}

void token_term(token_t* t)
{
    dstr_term(&(t->token_text));
}

int token_compose(token_t* t, token_type_t type, char const* ptr, dstr_len_t len)
{
    if (!dstr_assign_view(&(t->token_text), ptr, len))
        return 0;

    t->token_type = type;
    return 1;
}

