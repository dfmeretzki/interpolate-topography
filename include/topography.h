/*
    Filename: topography.h
    Author: David F. Meretzki
    Date: 2025-11-02

    Description:
    This file contains the declaration of the functions to handle topography data
*/

#ifndef TOPOGRAPHY_H
#define TOPOGRAPHY_H

#include <stddef.h>

#include "config_file.h"

typedef struct
{
    size_t nx;                  // number of x-values on the grid
    size_t ny;                  // number of y-values on the grid
    double* xGrid;              // x-topography grid
    double* yGrid;              // y-topography grid
    double* values;             // topography values
} Topography;

void freeTopography(Topography* topo);

int increaseTopographyResolution(const ConfigFile* config,
    const char* filename, Topography* topo);

#endif // TOPOGRAPHY_H
