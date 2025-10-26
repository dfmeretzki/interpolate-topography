/*
    Filename: mesh.c
    Author: David F. Meretzki
    Date: 2025-10-23

    Description:
    This file contains the definition of the mesh functions
*/

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "mesh.h"
#include "msh_constants.h"

static int markFaceNodes(const ConfigFile* config, Mesh* mesh)
{
    mesh->mark = (unsigned char*)calloc(mesh->nNodes, sizeof(unsigned char));
    if (mesh->mark == NULL)
    {
        fprintf(stderr, "Could not allocate memory for mark array of size %zu\n",
            mesh->nNodes);
        return 0;
    }

    mesh->triQuadCount = 0;
    mesh->maxElemNodes = 0;
    for (size_t index = 0; index < mesh->nElems; ++index)
    {
        unsigned int type = mesh->elements[index].type;
        if (type == MSH_TRI_3 || type == MSH_TRI_6 || type == MSH_QUA_4
            || type == MSH_QUA_8 || type == MSH_QUA_9)
        {
            mesh->triQuadCount += 1;
            if (mesh->elements[index].nNodes > mesh->maxElemNodes)
            {
                mesh->maxElemNodes = mesh->elements[index].nNodes;
            }

            for (int i = 0; i < MAXSURF; ++i)
            {
                int faceNum = config->surfaceMeshFaces[i];
                if (faceNum == 0) break;
                if (faceNum == mesh->elements[index].regElem)
                {
                    for (size_t j = 0; j < mesh->elements[index].nNodes; ++j)
                    {
                        size_t node = mesh->elements[index].nodes[j];
                        mesh->mark[node] = 1;
                    }
                }
            }
        }
    }

    return 1;
}

static int findInterval(const double* grid, size_t nGrid, double value, size_t* minIndex)
{
    if (value < grid[0] || value > grid[nGrid - 1]) return 0;

    size_t i = 0;
    size_t n = nGrid - 1;
    while (n - i > 1)
    {
        size_t mid = (i + n) / 2;
        if (grid[i] <= value && value < grid[mid])
        {
            n = mid;
        }
        else
        {
            i = mid;
        }
    }

    *minIndex = i;
    return 1;
}

static void moveNodes(const Topography* topo, Mesh* mesh)
{
    for (size_t i = 0; i < mesh->nNodes; ++i)
    {
        if (mesh->mark[i] == 0) continue;

        Node* node = &mesh->nodes[i];
        // Localize the node in the topography grid
        size_t ix, iy;
        if (!findInterval(topo->xGrid, topo->nx, node->x, &ix)) continue;
        if (!findInterval(topo->yGrid, topo->ny, node->y, &iy)) continue;

        // Perform Q1 interpolation
        double dx = topo->xGrid[ix + 1] - topo->xGrid[ix];
        double dy = topo->yGrid[iy + 1] - topo->yGrid[iy];
        double exi = 2.0 * ((node->x - topo->xGrid[ix]) / dx) - 1.0;
        double eta = 2.0 * ((node->y - topo->yGrid[iy]) / dy) - 1.0;
        double s1 = 1.0 - exi;
        double s2 = 1.0 + exi;
        double t1 = 1.0 - eta;
        double t2 = 1.0 + eta;
        double sh1 = s1 * t1;
        double sh2 = s2 * t1;
        double sh3 = s2 * t2;
        double sh4 = s1 * t2;
        double hi = (topo->values[iy * topo->nx + ix] * sh1
            + topo->values[iy * topo->nx + (ix + 1)] * sh2
            + topo->values[(iy + 1) * topo->nx + (ix + 1)] * sh3
            + topo->values[(iy + 1) * topo->nx + ix] * sh4) * 0.25;

        //TODO need both options? should be an execution flag or be in the config file?
        node->z = node->z + hi;
        //node->z = hi;
    }
}

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
    free(mesh->mark);
    mesh->mark = NULL;
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

int interpolateTopography(const ConfigFile* config, const Topography* topo, Mesh* mesh)
{
    if (!markFaceNodes(config, mesh)) return 0;

    moveNodes(topo, mesh);

    return 1;
}
