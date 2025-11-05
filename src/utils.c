/*
    Filename: utils.c
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains a collection definitions of common utility functions that
    are used throughout the Interpol Topography project
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

void removeSpaces(char* s)
{
    char* x = s;
    do
    {
        while (isspace(*x)) ++x;
    }
    while ((*s++ = *x++));
}

void combinePaths(char* dest, const char* path1, const char* path2)
{
    size_t len1 = strlen(path1);

    // Add separator if needed
    int needSeparator = 0;
    if (len1 > 0 && path1[len1 - 1] != PATH_SEPARATOR) needSeparator = 1;
    if (path2[0] == PATH_SEPARATOR) needSeparator = 0;

    if (needSeparator) sprintf(dest, "%s%c%s", path1, PATH_SEPARATOR, path2);
    else sprintf(dest, "%s%s", path1, path2);
}

void minMaxElement(const double* array, size_t n, double* min, double* max)
{
    *min = array[0];
    *max = array[0];
    for (size_t i = 1; i < n; ++i)
    {
        if (array[i] < *min)
        {
            *min = array[i];
        }
        if (array[i] > *max)
        {
            *max = array[i];
        }
    }
}
