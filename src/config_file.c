/*
    Filename: config_file.c
    Author: David F. Meretzki
    Date: 2025-10-20

    Description:
    This file contains the definition of the functions to read and parse the
    configuration file
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_file.h"
#include "utils.h"

static int parseLine(const char* restrict line, char* restrict key, char* restrict value)
{
    if (sscanf(line, "%[^=] = %[^\n]", key, value) == 2)
    {
        return 1;
    }
    else
    {
        printf("Format error, cannot extract key value from line: %s\n", line);
        return 0;
    }
}

static void parseArray(char* value, int* arr, int size)
{
    char* token = strtok(value, ",");
    int i = 0;
    while (token != NULL && i < size)
    {
        arr[i] = atoi(token);
        ++i;
        token = strtok(NULL, ",");
    }
}

static void parseStringArray(char* value, char arr[MAXSURF][MAX_PATH_LENGTH])
{
    char* token = strtok(value, ",");
    int i = 0;
    while (token != NULL && i < MAXSURF)
    {
        if (strlen(token) >= MAX_PATH_LENGTH)
        {
            printf("Error: '%s' file name too long, max length is %d\n", token, MAX_PATH_LENGTH - 1);
            exit(EXIT_FAILURE);
        }
        strcpy(arr[i], token);
        ++i;
        token = strtok(NULL, ",");
    }
}

static void storeValue(const char* restrict key, char* restrict value, ConfigFile* config)
{
    if (strcmp("skinMeshFileIn", key) == 0)
    {
        strcpy(config->skinMeshFileIn, value);
    }
    else if (strcmp("skinMeshFileOut", key) == 0)
    {
        strcpy(config->skinMeshFileOut, value);
    }
    else if (strcmp("topoFiles", key) == 0)
    {
        parseStringArray(value, config->topoFiles);
    }
    else if (strcmp("nx", key) == 0)
    {
        config->nx = (size_t)atoll(value);
    }
    else if (strcmp("ny", key) == 0)
    {
        config->ny = (size_t)atoll(value);
    }
    else if (strcmp("surfaceMeshFaces", key) == 0)
    {
        parseArray(value, config->surfaceMeshFaces, MAXSURF);
    }
    else if (strcmp("meshFacesToSmooth", key) == 0)
    {
        parseArray(value, config->meshFacesToSmooth, MAXSMOOTH);
    }
    else if (strcmp("iterMaxSmooth", key) == 0)
    {
        config->iterMaxSmooth = atoi(value);
    }
    else if (strcmp("tolerSmooth", key) == 0)
    {
        config->tolerSmooth = atof(value);
    }
    else
    {
        printf("Found unrecongnized key: %s", key);
    }
}

void validateConfigFile(const ConfigFile* config)
{
    if (config->skinMeshFileIn[0] == '\0')
    {
        fprintf(stderr, "Error: skinMeshFileIn not defined in config file\n");
        exit(EXIT_FAILURE);
    }
    if (config->skinMeshFileOut[0] == '\0')
    {
        fprintf(stderr, "Error: skinMeshFileOut not defined in config file\n");
        exit(EXIT_FAILURE);
    }

    int topoFileCount = 0;
    for (int i = 0; i < MAXSURF; ++i)
    {
        if (config->topoFiles[i][0] == '\0') break;
        ++topoFileCount;
    }
    int surfaceFaceCount = 0;
    for (int i = 0; i < MAXSURF; ++i)
    {
        if (config->surfaceMeshFaces[i] == 0) break;
        ++surfaceFaceCount;
    }

    if (topoFileCount == 0)
    {
        fprintf(stderr, "Error: topoFiles not defined in config file\n");
        exit(EXIT_FAILURE);
    }
    if (topoFileCount > 1 && topoFileCount != surfaceFaceCount)
    {
        fprintf(stderr, "Error: number of topoFiles (%d) does not match number of surfaceMeshFaces (%d)\n"
            "If more than one topography file is provided, there must be a corresponding surface face for each file.\n",
            topoFileCount, surfaceFaceCount);
        exit(EXIT_FAILURE);
    }
    if (surfaceFaceCount == 0)
    {
        fprintf(stderr, "Error: surfaceMeshFaces not defined in config file\n");
        exit(EXIT_FAILURE);
    }
    if (config->nx == 0)
    {
        fprintf(stderr, "Error: nx not defined in config file\n");
        exit(EXIT_FAILURE);
    }
    if (config->ny == 0)
    {
        fprintf(stderr, "Error: ny not defined in config file\n");
        exit(EXIT_FAILURE);
    }
    if (config->iterMaxSmooth <= 0)
    {
        fprintf(stderr, "Error: iterMaxSmooth must be greater than 0\n");
        exit(EXIT_FAILURE);
    }
    if (config->tolerSmooth <= 0.0)
    {
        fprintf(stderr, "Error: tolerSmooth must be greater than 0.0\n");
        exit(EXIT_FAILURE);
    }
}

void readConfigFile(const char* filename, ConfigFile* config)
{
    // set default values in case they are not defined
    config->iterMaxSmooth = 200;
    config->tolerSmooth = 0.01;

    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file '%s': %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char line[256];
    char key[256];
    char value[256];
    while (fgets(line, sizeof(line), file))
    {
        removeSpaces(line);
        if (strlen(line) == 0) continue;
        if (line[0] == '#') continue;
        if (parseLine(line, key, value)) storeValue(key, value, config);
    }

    fclose(file);

    validateConfigFile(config);
}

void printConfigFile(const ConfigFile* config)
{
    printf("\nConfig file:\n");
    printf("skinMeshFileIn = %s\n", config->skinMeshFileIn);
    printf("skinMeshFileOut = %s\n", config->skinMeshFileOut);
    printf("topoFiles = ");
    for (int i = 0; i < MAXSURF; ++i)
    {
        if (config->topoFiles[i][0] == '\0') break;
        if (i > 0) printf(", ");
        printf("%s", config->topoFiles[i]);
    }
    printf("\nnx = %zu\n", config->nx);
    printf("ny = %zu\n", config->ny);
    printf("surfaceMeshFaces = ");
    for (int i = 0; i < MAXSURF; ++i)
    {
        if (config->surfaceMeshFaces[i] == 0) break;
        if (i > 0) printf(", ");
        printf("%d", config->surfaceMeshFaces[i]);
    }
    printf("\nmeshFacesToSmooth = ");
    for (int i = 0; i < MAXSMOOTH; ++i)
    {
        if (config->meshFacesToSmooth[i] == 0) break;
        if (i > 0) printf(", ");
        printf("%d", config->meshFacesToSmooth[i]);
    }
    printf("\niterMaxSmooth = %d\n", config->iterMaxSmooth);
    printf("tolerSmooth = %lf\n", config->tolerSmooth);
}
