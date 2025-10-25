/*
    Filename: mesh.c
    Author: David F. Meretzki
    Date: 2025-10-23

    Description:
    This file contains the definition of the mesh functions
*/

#include <stdlib.h>

#include "mesh.h"

void freeMesh(Mesh* mesh)
{
    free(mesh->nodeIndex);
    mesh->nodeIndex = NULL;
    free(mesh->nodes);
    mesh->nodes = NULL;
    free(mesh->elemIndex);
    mesh->elemIndex = NULL;
    free(mesh->elements);
    mesh->elements = NULL;
}

void freeTopography(Topography* topo)
{
    free(topo->xGrid);
    topo->xGrid = NULL;
    free(topo->yGrid);
    topo->yGrid = NULL;
    free(topo->values);
    topo->values = NULL;
}
