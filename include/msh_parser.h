/*
    Filename: msh_parser.h
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains the declaration of the functions to parse MSH files. The
    MSH file format is the native mesh file format used by Gmsh
*/

#ifndef MSH_PARSER_H
#define MSH_PARSER_H

#include "mesh.h"

int readMshFile(const char* filename, Mesh* mesh);

#endif // MSH_PARSER_H