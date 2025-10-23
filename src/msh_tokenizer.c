/*
    Filename: msh_tokenizer.c
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains the definition of functions to tokenize a stream
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msh_tokenizer.h"

char errorBuffer[256];

typedef struct
{
    TokenType type;
    char* pattern;
} Spec;

Spec spec[] = {
    { TOKEN_V1_NOD_START,  "^\\$NOD" },
    { TOKEN_V1_NOD_END, "^\\$ENDNOD" },
    { TOKEN_V1_ELM_START, "^\\$ELM" },
    { TOKEN_V1_ELM_END, "^\\$ENDELM" },
    { TOKEN_NUMBER, "^[-+]?([0-9]+\\.[0-9]*|[0-9]+)" },
};


static int endOfFile(Tokenizer* tokenizer)
{
    return *tokenizer->current == '\0';
}

static char advance(Tokenizer* tokenizer)
{
    ++tokenizer->current;
    return tokenizer->current[-1];
}

static char peek(Tokenizer* tokenizer)
{
    return *tokenizer->current;
}

static void skipWhitespace(Tokenizer* tokenizer)
{
    while (isspace(peek(tokenizer)))
    {
        if (peek(tokenizer) == '\n') ++tokenizer->line;
        advance(tokenizer);
    }
}

static Token makeToken(const Tokenizer* tokenizer, TokenType type)
{
    Token token;
    token.type = type;
    token.start = tokenizer->start;
    token.length = (size_t)(tokenizer->current - tokenizer->start);
    token.line = tokenizer->line;
    return token;
}

static Token errorToken(const Tokenizer* tokenizer, const char* msg)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = msg;
    token.length = strlen(msg);
    token.line = tokenizer->line;
    return token;
}

static Token unexpectedCharacter(Tokenizer* tokenizer, const char c)
{
    int res = snprintf(
        errorBuffer,
        sizeof(errorBuffer),
        "Unexpected character '%c' at line %zd", c, tokenizer->line);
    if (res > 0) return errorToken(tokenizer, errorBuffer);
    else return errorToken(tokenizer, "Unexpected character");
}


void initTokenizer(Tokenizer* tokenizer, const char* source)
{
    tokenizer->source = source;
    tokenizer->start = source;
    tokenizer->current = source;
    tokenizer->line = 1;
    for (int i = 0; i < MSH_SPEC_SIZE; ++i)
    {
        if (regcomp(&tokenizer->specRegex[i], spec[i].pattern, REG_EXTENDED))
        {
            fprintf(stderr, "Could not compile regex %s", spec[i].pattern);
            exit(EXIT_FAILURE);
        }
    }
}

void freeTokenizer(Tokenizer* tokenizer)
{
    tokenizer->source = NULL;
    tokenizer->start = NULL;
    tokenizer->current = NULL;
    for (int i = 0; i < MSH_SPEC_SIZE; ++i)
    {
        regfree(&tokenizer->specRegex[i]);
    }
}

void resetTokenizer(Tokenizer* tokenizer, const char* source)
{
    tokenizer->source = source;
    tokenizer->start = source;
    tokenizer->current = source;
    tokenizer->line = 1;
}

Token nextToken(Tokenizer* tokenizer)
{
    skipWhitespace(tokenizer);
    tokenizer->start = tokenizer->current;

    if (endOfFile(tokenizer))
    {
        return makeToken(tokenizer, TOKEN_END_OF_FILE);
    }

    regmatch_t match[1];
    for (int i = 0; i < MSH_SPEC_SIZE; ++i)
    {
        if (regexec(&tokenizer->specRegex[i], tokenizer->start, 1, match, 0) == 0)
        {
            tokenizer->current = tokenizer->start + match[0].rm_eo;
            return makeToken(tokenizer, spec[i].type);
        }
    }

    return unexpectedCharacter(tokenizer, peek(tokenizer));
}

char* tokenTypeToValue(TokenType type)
{
    switch (type)
    {
    case TOKEN_V1_NOD_START:
        return "$NOD";
    case TOKEN_V1_NOD_END:
        return "$ENDNOD";
    case TOKEN_V1_ELM_START:
        return "$ELM";
    case TOKEN_V1_ELM_END:
        return "$ENDELM";
    case TOKEN_NUMBER:
        return "number";
    case TOKEN_END_OF_FILE:
        return "end of file";
    case TOKEN_ERROR:
        return "error";
    }
}
