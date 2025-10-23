/*
    Filename: mesh.h
    Author: David F. Meretzki
    Date: 2025-10-21

    Description:
    This file contains the declaration of the mesh and it's functions
*/

#ifndef MESH_H
#define MESH_H

#include <stddef.h>

#define MAX_ELEM_NODES 32

typedef struct
{
    double x;
    double y;
    double z;
} Node;

typedef struct
{
    int type;                       // geometrical type of the element
    int regPhys;                    // tag of the physical region
    int regElem;                    // tag of the element region
    size_t nNodes;                  // number of nodes of the element
    size_t nodes[MAX_ELEM_NODES];   // indexes of the nodes of the element
} Element;


typedef struct
{
    size_t nNodes;          // number of nodes in the mesh
    size_t* nodeIndex;      // index of each node in the mesh
    Node* nodes;            // array of nodes in the mesh
    size_t nElems;          // number of elements
    size_t* elemIndex;      // index of each element in the mesh
    Element* elements;      // array of elements in the mesh
} Mesh;

void freeMesh(Mesh* mesh);

#endif
