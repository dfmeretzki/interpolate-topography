/*
    Filename: background_mesh.h
    Author: David F. Meretzki
    Date: 2026-03-10

    Description:
    This file contains the declarations of the background mesh functions
*/

#ifndef BACKGROUND_MESH_H
#define BACKGROUND_MESH_H

#include "config_file.h"
#include "mesh.h"

int generateBackgroundMesh(const ConfigFile* config, const Mesh* mesh);

#endif // BACKGROUND_MESH_H
