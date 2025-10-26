/*
    Filename: msh_parser.c
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains the definition of functions to parse MSH files. The
    MSH file format is the native mesh file format used by Gmsh
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msh_parser.h"
#include "msh_tokenizer.h"

typedef struct
{
    Tokenizer* tokenizer;
    MSHVersion version;
    Token lookAhead;
    Token token;
} Parser;

static int readFileContent(const char* filename, char** content)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open .msh file '%s': %s\n", filename, strerror(errno));
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Could not seek to end of file '%s': %s\n", filename, strerror(errno));
        fclose(file);
        return 0;
    }

    long fileSize = ftell(file);
    if (fileSize == -1)
    {
        fprintf(stderr, "Could not get file size for '%s': %s\n", filename, strerror(errno));
        fclose(file);
        return 0;
    }

    rewind(file);
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Could not allocate memory for file content of '%s'\n", filename);
        fclose(file);
        return 0;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize)
    {
        fprintf(stderr, "Could not read file content of '%s'\n", filename);
        free(buffer);
        fclose(file);
        return 0;
    }

    buffer[bytesRead] = '\0';
    *content = buffer;

    fclose(file);
    return 1;
}

static MSHVersion detectMshVersion(Tokenizer* tokenizer)
{
    Token token = nextToken(tokenizer);
    if (token.type == TOKEN_V1_NOD_START
        || token.type == TOKEN_V1_ELM_END
        || token.type == TOKEN_V1_ELM_START
        || token.type == TOKEN_V1_NOD_END)
    {
        return MSH_V1;
    }
    return MSH_UNKNOWN_VERSION;
}

static int eatToken(Parser* parser, TokenType expectedType)
{
    if (parser->lookAhead.type != expectedType)
    {
        fprintf(stderr, "Expected %s at line %zu but found %s\n",
            tokenTypeToValue(expectedType),
            parser->lookAhead.line,
            tokenTypeToValue(parser->lookAhead.type));
        return 0;
    }

    parser->token = parser->lookAhead;
    parser->lookAhead = nextToken(parser->tokenizer);
    return 1;
}

static int parseNodStart(Parser* parser, Mesh* mesh)
{
    if (!eatToken(parser, TOKEN_V1_NOD_START)) return 0;

    // Read number of nodes
    if (!eatToken(parser, TOKEN_NUMBER))
    {
        fprintf(stderr, "Expected number of nodes at line %zu but found %.*s\n",
            parser->lookAhead.line,
            (int)parser->lookAhead.length,
            parser->lookAhead.start);
        return 0;
    }
    size_t nNodes = (size_t)atoll(parser->token.start);

    // Allocate memory for nodes
    mesh->nNodes = nNodes;
    mesh->nodeIndex = (size_t*)malloc(nNodes * sizeof(size_t));
    if (mesh->nodeIndex == NULL)
    {
        fprintf(stderr, "Could not allocate memory for %zu node indexes\n", nNodes);
        return 0;
    }
    mesh->nodes = (Node*)malloc(nNodes * sizeof(Node));
    if (mesh->nodes == NULL)
    {
        fprintf(stderr, "Could not allocate memory for %zu nodes\n", nNodes);
        return 0;
    }

    // Read node data
    for (size_t i = 0; i < nNodes; ++i)
    {
        // Read node index
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;

        // Subtract 1 to convert to 0-based index
        size_t nodeIndex = (size_t)atoll(parser->token.start) - 1;
        if (nodeIndex < 0 || nodeIndex >= nNodes)
        {
            fprintf(stderr, "Node index %zu out of bounds at line %zu\n",
                nodeIndex + 1,
                parser->token.line);
            return 0;
        }

        // Read x coordinate
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        double x = atof(parser->token.start);

        // Read y coordinate
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        double y = atof(parser->token.start);

        // Read z coordinate
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        double z = atof(parser->token.start);

        // Store node data
        mesh->nodeIndex[i] = nodeIndex;
        mesh->nodes[nodeIndex].x = x;
        mesh->nodes[nodeIndex].y = y;
        mesh->nodes[nodeIndex].z = z;
    }

    if (!eatToken(parser, TOKEN_V1_NOD_END))
    {
        return 0;
    }

    return 1;
}

static int parseElmStart(Parser* parser, Mesh* mesh)
{
    if (!eatToken(parser, TOKEN_V1_ELM_START)) return 0;

    // Read number of elements
    if (!eatToken(parser, TOKEN_NUMBER))
    {
        fprintf(stderr, "Expected number of elements at line %zu but found %.*s\n",
            parser->lookAhead.line,
            (int)parser->lookAhead.length,
            parser->lookAhead.start);
        return 0;
    }
    size_t nElems = (size_t)atoll(parser->token.start);

    // Allocate memory for elements
    mesh->nElems = nElems;
    mesh->elemIndex = (size_t*)malloc(nElems * sizeof(size_t));
    if (mesh->elemIndex == NULL)
    {
        fprintf(stderr, "Could not allocate memory for %zu element indexes\n", nElems);
        return 0;
    }
    mesh->elements = (Element*)malloc(nElems * sizeof(Element));
    if (mesh->elements == NULL)
    {
        fprintf(stderr, "Could not allocate memory for %zu elements\n", nElems);
        return 0;
    }

    // Read element data
    for (size_t i = 0; i < nElems; ++i)
    {
        // Read element index
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;

        // Subtract 1 to convert to 0-based index
        size_t elemIndex = (size_t)atoll(parser->token.start) - 1;
        if (elemIndex < 0 || elemIndex >= nElems)
        {
            fprintf(stderr, "Element index %zu out of bounds at line %zu\n",
                elemIndex + 1,
                parser->token.line);
            return 0;
        }

        // Read element type
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        unsigned int type = atoi(parser->token.start);

        // Read physical region
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        unsigned int regPhys = atoi(parser->token.start);

        // Read element region
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        unsigned int regElem = atoi(parser->token.start);

        // Read number of nodes
        if (!eatToken(parser, TOKEN_NUMBER)) return 0;
        size_t nNodes = (size_t)atoll(parser->token.start);

        // Read node indexes
        for (size_t j = 0; j < nNodes; ++j)
        {
            if (!eatToken(parser, TOKEN_NUMBER)) return 0;
            // Subtract 1 to convert to 0-based index
            mesh->elements[elemIndex].nodes[j] = (size_t)atoll(parser->token.start) - 1;
        }

        // Store element data
        mesh->elemIndex[i] = elemIndex;
        mesh->elements[elemIndex].type = type;
        mesh->elements[elemIndex].regPhys = regPhys;
        mesh->elements[elemIndex].regElem = regElem;
        mesh->elements[elemIndex].nNodes = nNodes;
    }

    if (!eatToken(parser, TOKEN_V1_ELM_END))
    {
        return 0;
    }

    return 1;
}

static int parseMshV1(Parser* parser, Mesh* mesh)
{
    parser->lookAhead = nextToken(parser->tokenizer);
    while (parser->lookAhead.type != TOKEN_END_OF_FILE)
    {
        switch (parser->lookAhead.type)
        {
        case TOKEN_V1_NOD_START:
            if (!parseNodStart(parser, mesh)) return 0;
            break;
        case TOKEN_V1_ELM_START:
            if (!parseElmStart(parser, mesh)) return 0;
            break;
        default:
            fprintf(stderr, "Expected %s or %s at line %zu but found %s\n",
                tokenTypeToValue(TOKEN_V1_NOD_START),
                tokenTypeToValue(TOKEN_V1_ELM_START),
                parser->lookAhead.line,
                tokenTypeToValue(parser->lookAhead.type));
            return 0;
        }
    }

    return 1;
}

int readMshFile(const char* filename, Mesh* mesh)
{
    char* buffer = NULL;
    if (!readFileContent(filename, &buffer))
    {
        return 0;
    }

    Tokenizer tokenizer;
    initTokenizer(&tokenizer, buffer);

    Parser parser;
    parser.tokenizer = &tokenizer;
    parser.version = detectMshVersion(&tokenizer);
    resetTokenizer(&tokenizer, buffer);
    int result = 0;
    switch (parser.version)
    {
    case MSH_V1:
        result = parseMshV1(&parser, mesh);
        break;
    case MSH_UNKNOWN_VERSION:
    default:
        fprintf(stderr, "Unsupported or unknown MSH version in file '%s'\n", filename);
    }

    if (!result) freeMesh(mesh);
    free(buffer);
    freeTokenizer(&tokenizer);
    return result;
}
