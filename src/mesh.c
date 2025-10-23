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
    free(mesh->nodes);
    free(mesh->elemIndex);
    free(mesh->elements);
}
