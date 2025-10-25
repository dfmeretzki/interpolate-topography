/*
    Filename: topography_parser.c
    Author: David F. Meretzki
    Date: 2025-10-25

    Description:
    This file contains the implementation of the functions to parse topography files
*/

#include <stdio.h>
#include <stdlib.h>

#include "topography_parser.h"

int readTopographyFile(const char* filename, Topography* topo)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open topography file: %s\n", filename);
        return 0;
    }

    // Read grid dimensions
    if (fscanf(file, "%zu %zu", &topo->nx, &topo->ny) != 2)
    {
        fprintf(stderr, "Error reading dimensions from topography file: %s\n", filename);
        fclose(file);
        return 0;
    }

    topo->xGrid = (double*)malloc(topo->nx * sizeof(double));
    if (topo->xGrid == NULL)
    {
        fprintf(stderr, "Could not allocate memory for x grid\n");
        fclose(file);
        return 0;
    }
    topo->yGrid = (double*)malloc(topo->ny * sizeof(double));
    if (topo->yGrid == NULL)
    {
        fprintf(stderr, "Could not allocate memory for y grid\n");
        freeTopography(topo);
        fclose(file);
        return 0;
    }
    topo->values = (double*)malloc(topo->nx * topo->ny * sizeof(double));
    if (topo->values == NULL)
    {
        fprintf(stderr, "Could not allocate memory for topography values\n");
        freeTopography(topo);
        fclose(file);
        return 0;
    }

    // Read x grid values
    for (size_t i = 0; i < topo->nx; ++i)
    {
        if (fscanf(file, "%lf", &topo->xGrid[i]) != 1)
        {
            fprintf(stderr, "Error reading x grid data from topography file: %s at index %zu\n",
                filename, i);
            freeTopography(topo);
            fclose(file);
            return 0;
        }
    }

    // Read y grid values
    for (size_t i = 0; i < topo->ny; ++i)
    {
        if (fscanf(file, "%lf", &topo->yGrid[i]) != 1)
        {
            fprintf(stderr, "Error reading y grid data from topography file: %s at index %zu\n",
                filename, i);
            freeTopography(topo);
            fclose(file);
            return 0;
        }
    }

    // Read topography values
    for (size_t i = 0; i < topo->ny; ++i)
    {
        for (size_t j = 0; j < topo->nx; ++j)
        {
            if (fscanf(file, "%lf", &topo->values[i * topo->nx + j]) != 1)
            {
                fprintf(stderr, "Error reading topography values from topography file: "
                    "%s at index %zu\n",
                    filename, i * topo->nx + j);
                freeTopography(topo);
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return 1;
}
