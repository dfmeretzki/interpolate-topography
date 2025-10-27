/*
    Filename: msh_tokenizer.h
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains the declaration of the tokens and the functions to
    tokenize a stream
*/

#ifndef MSH_TOKENIZER_H
#define MSH_TOKENIZER_H

#include <regex.h>
#include <stddef.h>

typedef enum
{
    MSH_V1,
    MSH_UNKNOWN_VERSION
} MSHVersion;

typedef enum
{
    TOKEN_V1_NOD_START,         // $NOD
    TOKEN_V1_NOD_END,           // $ENDNOD
    TOKEN_V1_ELM_START,         // $ELM
    TOKEN_V1_ELM_END,           // $ENDELM
    TOKEN_NUMBER,
    // add new token types above this line
    MSH_SPEC_SIZE,              // number of token types in the spec
    TOKEN_END_OF_FILE,
    TOKEN_ERROR
} TokenType;

typedef struct
{
    TokenType type;
    const char* start;
    size_t length;
    size_t line;
} Token;

typedef struct
{
    const char* source;
    const char* start;
    const char* current;
    size_t line;
    regex_t specRegex[MSH_SPEC_SIZE];
} Tokenizer;

void initTokenizer(Tokenizer* tokenizer, const char* source);

void freeTokenizer(Tokenizer* tokenizer);

void resetTokenizer(Tokenizer* tokenizer, const char* source);

Token nextToken(Tokenizer* tokenizer, TokenType hint);

char* tokenTypeToValue(TokenType type);

#endif // MSH_TOKENIZER_H