/*
    Filename: msh_tokenizer_tests.c
    Author: David F. Meretzki
    Date: 2025-10-22

    Description:
    This file contains the tests for the msh tokenizer
*/


#include <stdio.h>

#include "msh_tokenizer.h"

static char* typeToString(TokenType type)
{
    switch (type)
    {
    case TOKEN_V1_NOD_START:
        return "TOKEN_V1_NOD_START";
    case TOKEN_V1_NOD_END:
        return "TOKEN_V1_NOD_END";
    case TOKEN_V1_ELM_START:
        return "TOKEN_V1_ELM_START";
    case TOKEN_V1_ELM_END:
        return "TOKEN_V1_ELM_END";
    case TOKEN_NUMBER:
        return "TOKEN_NUMBER";
    case TOKEN_END_OF_FILE:
        return "TOKEN_END_OF_FILE";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    }
}

static int compare(const char* restrict x, const char* restrict y, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        if (x[i] != y[i]) return 0;
    }

    return 1;
}

static int tokenizerTests()
{
    {
        // Test tokens of MSH format v1
        Tokenizer tokenizer;
        char* file = "\n $NOD12 -14.5 15$ENDNOD  \n$ELM$ENDELM   ";
        TokenType r[] = { TOKEN_V1_NOD_START, TOKEN_NUMBER, TOKEN_NUMBER, TOKEN_NUMBER,
            TOKEN_V1_NOD_END, TOKEN_V1_ELM_START, TOKEN_V1_ELM_END, TOKEN_END_OF_FILE };
        initTokenizer(&tokenizer, file);
        Token t;
        int i = 0;
        do
        {
            t = nextToken(&tokenizer);
            if (t.type != r[i])
            {
                printf("Found token type %s while expecting %s",
                    typeToString(t.type), typeToString(r[i]));
                return 1;
            }
            ++i;
        }
        while (t.type != TOKEN_END_OF_FILE);
        freeTokenizer(&tokenizer);
    }
    {
        // Test token value extraction
        Tokenizer tokenizer;
        char* file = "$NOD\n123 -45.67$ENDNOD$ELM0.001 100\n    $ENDELM";
        char* r[] = { "$NOD", "123", "-45.67", "$ENDNOD", "$ELM", "0.001", "100", "$ENDELM", "\0" };
        initTokenizer(&tokenizer, file);
        Token t;
        int i = 0;
        do
        {
            t = nextToken(&tokenizer);
            if (compare(t.start, r[i], t.length) != 1)
            {
                printf("Extracted token value %.*s while expecting %s",
                    (int)t.length, t.start, r[i]);
                return 1;
            }
            ++i;
        }
        while (t.type != TOKEN_END_OF_FILE);
        freeTokenizer(&tokenizer);
    }
    {
        // Test line counting
        Tokenizer tokenizer;
        char* file = "\n$NOD\n123\n-45.67\n\n\n$ENDNOD\n$ELM\n0.001\n\n100\n$ENDELM";
        size_t r[] = { 2, 3, 4, 7, 8, 9, 11, 12, 12 };
        initTokenizer(&tokenizer, file);
        Token t;
        int i = 0;
        do
        {
            t = nextToken(&tokenizer);
            if (t.line != r[i])
            {
                printf("Found token line %zu while expecting %zu", t.line, r[i]);
                return 1;
            }
            ++i;
        }
        while (t.type != TOKEN_END_OF_FILE);
        freeTokenizer(&tokenizer);
    }
    {
        // Test token error
        Tokenizer tokenizer;
        char* file = "$NOD @123";
        initTokenizer(&tokenizer, file);
        Token t = nextToken(&tokenizer);    // $NOD
        t = nextToken(&tokenizer);          // @123
        if (t.type != TOKEN_ERROR)
        {
            printf("Expected TOKEN_ERROR but found %s", typeToString(t.type));
            return 1;
        }
    }
    {
        // Test reset tokenizer
        Tokenizer tokenizer;
        char* file1 = "$NOD 1 2 3 $ENDNOD";
        char* file2 = "$ELM 4 5 6 $ENDELM";
        initTokenizer(&tokenizer, file1);
        Token t1 = nextToken(&tokenizer); // $NOD
        Token t2 = nextToken(&tokenizer); // 1
        resetTokenizer(&tokenizer, file2);
        Token t3 = nextToken(&tokenizer); // $ELM
        if (t1.type != TOKEN_V1_NOD_START || t2.type != TOKEN_NUMBER
            || t3.type != TOKEN_V1_ELM_START)
        {
            printf("Tokenizer reset failed");
            return 1;
        }
        freeTokenizer(&tokenizer);
    }
    {
        // Test empty input
        Tokenizer tokenizer;
        char* file = "";
        initTokenizer(&tokenizer, file);
        Token t = nextToken(&tokenizer);
        if (t.type != TOKEN_END_OF_FILE)
        {
            printf("Expected TOKEN_END_OF_FILE but found %s", typeToString(t.type));
            return 1;
        }
        freeTokenizer(&tokenizer);
    }
    {
        // Test input with only whitespace
        Tokenizer tokenizer;
        char* file = "   \n\t  \n  ";
        initTokenizer(&tokenizer, file);
        Token t = nextToken(&tokenizer);
        if (t.type != TOKEN_END_OF_FILE)
        {
            printf("Expected TOKEN_END_OF_FILE but found %s", typeToString(t.type));
            return 1;
        }
        freeTokenizer(&tokenizer);
    }
    {
        // Test invalid token at the start
        Tokenizer tokenizer;
        char* file = "$$NOD 1 2 3 $ENDNOD";
        initTokenizer(&tokenizer, file);
        Token t = nextToken(&tokenizer); // $
        if (t.type != TOKEN_ERROR)
        {
            printf("Expected TOKEN_ERROR but found %s", typeToString(t.type));
            return 1;
        }
        freeTokenizer(&tokenizer);
    }
    {
        // Test sequence of numbers
        Tokenizer tokenizer;
        char* file = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20";
        initTokenizer(&tokenizer, file);
        for (int i = 0; i <= 20; ++i)
        {
            Token t = nextToken(&tokenizer);
            if (t.type != TOKEN_NUMBER)
            {
                printf("Expected TOKEN_NUMBER but found %s", typeToString(t.type));
                return 1;
            }
            char expected[3];
            snprintf(expected, sizeof(expected), "%d", i);
            if (compare(t.start, expected, t.length) != 1)
            {
                printf("Extracted token value %.*s while expecting %s",
                    (int)t.length, t.start, expected);
                return 1;
            }
        }
        Token t = nextToken(&tokenizer);
        if (t.type != TOKEN_END_OF_FILE)
        {
            printf("Expected TOKEN_END_OF_FILE but found %s", typeToString(t.type));
            return 1;
        }
        freeTokenizer(&tokenizer);
    }

    return 0;
}

int main()
{
    return tokenizerTests();
}
