/*
    Filename: config_file.h
    Author: David F. Meretzki
    Date: 2025-10-20

    Description:
    This file contains the definition of the configuration file and the
    functions to read and parse the file
*/

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "constants.h"

typedef struct
{
    char skinMeshFileIn[MAX_PATH_LENGTH];   // the input mesh file name
    char skinMeshFileOut[MAX_PATH_LENGTH];  // the output mesh file name
    char topoFile[MAX_PATH_LENGTH];         // the topography file (grid) name
    int surfaceMeshFaces[MAXSURF];          // the face #(s) corresponding to the surface
    int meshFacesToSmooth[MAXSMOOTH];       // the face #(s) where barycentric smoothing will be applied if desired

    // Used in the smoothing algorithm
    int iterMaxSmooth;                      // default value = 0
    double tolerSmooth;                     // default value = 0.0
} ConfigFile;

void readConfigFile(const char* filename, ConfigFile* config);

void printConfigFile(const ConfigFile* config);

#endif