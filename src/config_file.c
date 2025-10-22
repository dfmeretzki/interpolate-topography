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
        removeSpaces(key);
        removeSpaces(value);
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
    else if (strcmp("topoFile", key) == 0)
    {
        strcpy(config->topoFile, value);
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

void readConfigFile(const char* filename, ConfigFile* config)
{
    // set default values in case they are not defined
    config->iterMaxSmooth = 0;
    config->tolerSmooth = 0.0;

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
        if (line[0] == '#' || line[0] == '\n') continue;
        if (parseLine(line, key, value)) storeValue(key, value, config);
    }

    fclose(file);
}

void printConfigFile(const ConfigFile* config)
{
    printf("\nConfig file:\n");
    printf("skinMeshFileIn = %s\n", config->skinMeshFileIn);
    printf("skinMeshFileOut = %s\n", config->skinMeshFileOut);
    printf("topoFile = %s\n", config->topoFile);
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
