/*
    Filename: resistivity_parser.h
    Author: David F. Meretzki
    Date: 2026-03-05

    Description:
    This file contains the declaration of the functions to parse resistivity files
*/

#ifndef RESISTIVITY_PARSER_H
#define RESISTIVITY_PARSER_H

#include "resistivity.h"

int readSEGYFile(const char* filename, Resistivity* res);

#endif // RESISTIVITY_PARSER_H
