/*
    Filename: config_file.h
    Author: David F. Meretzki
    Date: 2025-10-20

    Description:
    This file contains the declaration of the configuration file and the
    functions to read and parse the file
*/

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <stddef.h>
#include <stdint.h>

#include "constants.h"

enum ConfigMode : uint8_t
{
    MODE_INTERPOLATE = 1 << 0,
    MODE_BACKGROUND_MESH = 1 << 1,
    MODE_ALL = MODE_INTERPOLATE | MODE_BACKGROUND_MESH
};

typedef struct
{
    enum ConfigMode mode;                       // the mode of operation
    char skinMeshFileIn[MAX_PATH_LENGTH];       // the input mesh file name
    char skinMeshFileOut[MAX_PATH_LENGTH];      // the output mesh file name
    char topoFiles[MAXSURF][MAX_PATH_LENGTH];   // the topography files (grid) names
    size_t nx;                                  // number of x-values to use on the grid
    size_t ny;                                  // number of y-values to use on the grid
    int surfaceMeshFaces[MAXSURF];              // the face #(s) corresponding to the surface
    int meshFacesToSmooth[MAXSMOOTH];           // the face #(s) where barycentric smoothing will be applied if desired

    // Used in the smoothing algorithm
    int iterMaxSmooth;                          // default value = 200
    double tolerSmooth;                         // default value = 0.01

    // Used in the background mesh generation
    double minResistivity;                       // the minimum resistivity value, if not defined it will be calculated from the resistivity file
    char resistivityFile[MAX_PATH_LENGTH];       // the input resistivity file name
    char sourcesFile[MAX_PATH_LENGTH];           // the input sources/receivers file name
    char backgroundMeshFile[MAX_PATH_LENGTH];    // the output background mesh file name
    double frequency;                            // default value = 1.0
    double rSkinDepth;                           // default value = 2.0
    double emitterLength;                        // default value = 1.0
    double rsFactor;                             // default value = 10.0
    double growthFactor;                         // default value = 1.25
} ConfigFile;

void readConfigFile(const char* filename, ConfigFile* config);

void printConfigFile(const ConfigFile* config);

#endif