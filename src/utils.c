/*
    Filename: utils.c
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains a collection definitions of common utility functions that
    are used throughout the Interpol Topography project
*/

#include <ctype.h>

#include "utils.h"

void removeSpaces(char* s)
{
    char* x = s;
    do
    {
        while (isspace(*x)) ++x;
    }
    while ((*s++ = *x++));
}
