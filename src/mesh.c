/*
    Filename: mesh.c
    Author: David F. Meretzki
    Date: 2025-10-23

    Description:
    This file contains the definition of the mesh functions
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "mesh.h"
#include "msh_constants.h"

typedef struct
{
    size_t nodes[MAXCN];           // array of unique connected nodes
    size_t nConnections;           // number of connected nodes
    size_t totalConnections;       // number of total connections
} NodeConnections;

static int markFaceNodes(unsigned int face, Mesh* mesh)
{
    if (mesh->mark == NULL)
    {
        mesh->mark = (unsigned char*)calloc(mesh->nNodes, sizeof(unsigned char));
        if (mesh->mark == NULL)
        {
            fprintf(stderr, "Could not allocate memory for mark array of size %zu\n",
                mesh->nNodes);
            return 0;
        }
    }
    else
    {
        memset(mesh->mark, 0, mesh->nNodes * sizeof(unsigned char));
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
            if (face == mesh->elements[index].regElem)
            {
                for (size_t j = 0; j < mesh->elements[index].nNodes; ++j)
                {
                    size_t node = mesh->elements[index].nodes[j];
                    mesh->mark[node] = 1;
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


static int findNode(const size_t* arr, size_t size, size_t value)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (arr[i] == value) return 1;
    }

    return 0;
}

static int getNodeConnections(unsigned int face, Mesh* mesh, NodeConnections* nodeConns)
{
    if (mesh->mark == NULL)
    {
        mesh->mark = (unsigned char*)calloc(mesh->nNodes, sizeof(unsigned char));
        if (mesh->mark == NULL)
        {
            fprintf(stderr, "Could not allocate memory for mark array of size %zu\n",
                mesh->nNodes);
            return 0;
        }
    }
    else
    {
        memset(mesh->mark, 0, mesh->nNodes * sizeof(unsigned char));
    }

    for (size_t index = 0; index < mesh->nElems; ++index)
    {
        // Skip elements that are not tri or quad
        unsigned int type = mesh->elements[index].type;
        if (type != MSH_TRI_3 && type != MSH_TRI_6 && type != MSH_QUA_4
            && type != MSH_QUA_8 && type != MSH_QUA_9) continue;

        // Skip elements that doesn't belong to the face
        if (face != mesh->elements[index].regElem) continue;

        size_t nNodes = mesh->elements[index].nNodes;
        for (size_t i = 0; i < nNodes; ++i)
        {
            size_t node = mesh->elements[index].nodes[i];
            mesh->mark[node] = 1;
            nodeConns[node].totalConnections += 1;
            for (size_t j = 0; j < nNodes; ++j)
            {
                // same node, skip
                if (i == j) continue;

                size_t n = mesh->elements[index].nodes[j];
                if (!findNode(nodeConns[node].nodes, nodeConns[node].nConnections, n))
                {
                    nodeConns[node].nodes[nodeConns[node].nConnections] = n;
                    nodeConns[node].nConnections += 1;
                    if (nodeConns[node].nConnections > MAXCN)
                    {
                        fprintf(stderr, "Exceeded maximum number of connections %d for node %zu\n",
                            MAXCN, node);
                        return 0;
                    }
                }
            }
        }
    }

    return 1;
}

static void smoothFace(int face, int nIterMax, double toler,
    const NodeConnections* nodeConns, Mesh* mesh)
{
    double dep = 0.0;
    double dep1 = 0.0;
    int converged = 0;
    int iter;
    for (iter = 0; iter < nIterMax; ++iter)
    {
        dep = 0.0;
        size_t nodeCount = 0;
        for (size_t nId = 0; nId < mesh->nNodes; ++nId)
        {
            // Skip nodes that doesn't belong to the face
            if (mesh->mark[nId] == 0) continue;

            // Skip boundary nodes
            if (nodeConns[nId].nConnections != nodeConns[nId].totalConnections) continue;

            Node nodeSum = { 0 };
            for (size_t i = 0; i < nodeConns[nId].nConnections; ++i)
            {
                size_t connId = nodeConns[nId].nodes[i];
                Node connNode = mesh->nodes[connId];
                nodeSum.x += connNode.x;
                nodeSum.y += connNode.y;
                nodeSum.z += connNode.z;
            }
            nodeSum.x /= (double)nodeConns[nId].nConnections;
            nodeSum.y /= (double)nodeConns[nId].nConnections;
            nodeSum.z /= (double)nodeConns[nId].nConnections;

            Node* node = &mesh->nodes[nId];
            Node v = { nodeSum.x - node->x, nodeSum.y - node->y, nodeSum.z - node->z };
            dep += v.x * v.x + v.y * v.y + v.z * v.z;

            node->x = nodeSum.x;
            node->y = nodeSum.y;
            node->z = nodeSum.z;
            ++nodeCount;
        }
        dep = sqrt(dep) / (double)nodeCount;

        if (iter == 0) dep1 = dep;
        else if (dep < toler * dep1)
        {
            converged = 1;
            break;
        }
    }

    if (converged)
    {
        printf("Smoothing for face #%d converged in %d iterations\n", face, iter);
    }
    else
    {
        printf("Smoothing for face #%d NOT converged in %d iterations (dep/dep1) = %lf\n",
            face, iter, dep / dep1);
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

int interpolateTopography(const ConfigFile* config, const Topography* topo, Mesh* mesh)
{
    for (size_t i = 0; i < MAXSURF; ++i)
    {
        if (config->surfaceMeshFaces[i] == 0) break;

        if (!markFaceNodes(config->surfaceMeshFaces[i], mesh)) return 0;

        moveNodes(topo, mesh);
    }

    return 1;
}

int interpolate(const ConfigFile* config, Mesh* mesh)
{
    int topoFilesCount = 0;
    for (int i = 0; i < MAXSURF; ++i)
    {
        if (config->topoFiles[i][0] == '\0') break;
        ++topoFilesCount;
    }

    int result = 1;
    Topography topo = { 0 };
    for (int i = 0; i < MAXSURF; ++i)
    {
        if (config->surfaceMeshFaces[i] == 0) break;

        if (!markFaceNodes(config->surfaceMeshFaces[i], mesh))
        {
            result = 0;
            goto out_free_topo;
        }

        if (i < topoFilesCount)
        {
            freeTopography(&topo);
            if (!increaseTopographyResolution(config, config->topoFiles[i], &topo))
            {
                result = 0;
                goto out_free_topo;
            }
        }

        moveNodes(&topo, mesh);
    }

out_free_topo:
    freeTopography(&topo);
    return result;
}

int smoothMesh(const ConfigFile* config, Mesh* mesh)
{
    // No faces to smooth
    if (config->meshFacesToSmooth[0] == 0) return 1;

    int nIterMax = config->iterMaxSmooth;
    double toler = config->tolerSmooth;
    if (config->iterMaxSmooth == 0) nIterMax = 200;
    if (config->tolerSmooth == 0.0) toler = 0.01;

    NodeConnections* lNodes = (NodeConnections*)malloc(mesh->nNodes * sizeof(NodeConnections));
    if (lNodes == NULL)
    {
        fprintf(stderr, "Could not allocate memory for node connections array of size %zu\n",
            mesh->nNodes);
        return 0;
    }

    for (int i = 0; i < MAXSMOOTH; ++i)
    {
        unsigned int faceNum = config->meshFacesToSmooth[i];
        if (faceNum == 0) break;

        memset(lNodes, 0, mesh->nNodes * sizeof(NodeConnections));
        if (!getNodeConnections(faceNum, mesh, lNodes))
        {
            free(lNodes);
            return 0;
        }

        smoothFace(faceNum, nIterMax, toler, lNodes, mesh);
    }

    free(lNodes);

    return 1;
}
