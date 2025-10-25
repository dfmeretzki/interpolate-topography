/*
    Filename: topography_parser.h
    Author: David F. Meretzki
    Date: 2025-10-25

    Description:
    This file contains the declaration of the functions to parse topography files
*/

#ifndef TOPOGRAPHY_PARSER_H
#define TOPOGRAPHY_PARSER_H

#include "mesh.h"

int readTopographyFile(const char* filename, Topography* topo);

#endif // TOPOGRAPHY_PARSER_H