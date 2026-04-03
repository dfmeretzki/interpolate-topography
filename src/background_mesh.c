/*
    Filename: background_mesh.c
    Author: David F. Meretzki
    Date: 2026-03-10

    Description:
    This file contains the definitions of the background mesh functions
*/

#include "background_mesh.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "resistivity_parser.h"
#include "resistivity.h"
#include "topography_parser.h"
#include "utils.h"

typedef struct NodeBin
{
    size_t id;
    struct NodeBin* next;
} NodeBin;

typedef struct
{
    int nx, ny, nz;
    float minX, minY, minZ;
    float h;
    NodeBin** bins;
} BinGrid;

static size_t binIndex(const BinGrid* grid, int x, int y, int z)
{
    return (size_t)((z * grid->ny + y) * grid->nx + x);
}

static void freeBinGrid(BinGrid* grid)
{
    for (size_t i = 0; i < (size_t)grid->nx * grid->ny * grid->nz; ++i)
    {
        NodeBin* node = grid->bins[i];
        while (node)
        {
            NodeBin* next = node->next;
            free(node);
            node = next;
        }
    }
    free(grid->bins);
    grid->bins = NULL;
}

static int buildBinGrid(BinGrid* grid, const Node* nodes, size_t nNodes,
    float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float radius)
{
    int result = 1;

    grid->minX = minX;
    grid->minY = minY;
    grid->minZ = minZ;
    grid->h = radius;
    grid->nx = (int)floorf((maxX - minX) / radius) + 1;
    grid->ny = (int)floorf((maxY - minY) / radius) + 1;
    grid->nz = (int)floorf((maxZ - minZ) / radius) + 1;

    size_t nBins = (size_t)grid->nx * grid->ny * grid->nz;
    grid->bins = (NodeBin**)calloc(nBins, sizeof(NodeBin*));
    if (!grid->bins) return 0;

    for (size_t n = 0; n < nNodes; ++n)
    {
        int ix = clampi((int)floorf((nodes[n].x - grid->minX) / grid->h), 0, grid->nx - 1);
        int iy = clampi((int)floorf((nodes[n].y - grid->minY) / grid->h), 0, grid->ny - 1);
        int iz = clampi((int)floorf((nodes[n].z - grid->minZ) / grid->h), 0, grid->nz - 1);
        int index = binIndex(grid, ix, iy, iz);

        NodeBin* node = (NodeBin*)malloc(sizeof(NodeBin));
        if (!node)
        {
            fprintf(stderr, "Could not allocate memory for NodeBin\n");
            result = 0;
            goto free_grid;
        }
        node->id = n;
        node->next = grid->bins[index];
        grid->bins[index] = node;
    }

    goto out;

free_grid:
    freeBinGrid(grid);
out:
    return result;
}

static float elementSize(const BinGrid* grid, const Node* nodes, float growthFactor,
    float x, float y, float z, float skinSize, float sourceSize)
{
    if (grid->bins == NULL) return skinSize;

    float size = skinSize;

    int bix = clampi((int)floorf((x - grid->minX) / grid->h), 0, grid->nx - 1);
    int biy = clampi((int)floorf((y - grid->minY) / grid->h), 0, grid->ny - 1);
    int biz = clampi((int)floorf((z - grid->minZ) / grid->h), 0, grid->nz - 1);

    for (int dz = -1; dz <= 1; ++dz)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                int ix = bix + dx, iy = biy + dy, iz = biz + dz;
                if (ix < 0 || iy < 0 || iz < 0
                    || ix >= grid->nx || iy >= grid->ny || iz >= grid->nz) continue;

                for (NodeBin* b = grid->bins[binIndex(grid, ix, iy, iz)]; b; b = b->next)
                {
                    const Node* n = &nodes[b->id];
                    float d = (float)fmax(fabs(x - n->x), fmax(fabs(y - n->y), fabs(z - n->z)));
                    if (d > grid->h) continue;

                    int layer = (int)floorf(d / skinSize);
                    float sSize = sourceSize * powf(growthFactor, (float)layer);
                    if (sSize < size) size = sSize;
                }
            }
        }
    }

    return size;
}

static int initResistivity(const ConfigFile* config, const Mesh* mesh, Resistivity* res)
{
    if (isnan(config->minResistivity))
    {
        if (!readSEGYMinResistivity(config->resistivityFile, &res->minResistivity))
        {
            fprintf(stderr, "Failed to read resistivity file '%s'\n", config->resistivityFile);
            return 0;
        }
    }
    else
    {
        res->minResistivity = (float)config->minResistivity;
    }

    getShape(mesh, &res->minX, &res->maxX, &res->minY, &res->maxY, &res->minZ, &res->maxZ);
    res->dx = 50.0f;
    res->dy = 50.0f;
    res->dz = 50.0f;
    res->nx = (int)floorf((res->maxX - res->minX) / res->dx) + 1;
    res->ny = (int)floorf((res->maxY - res->minY) / res->dy) + 1;
    res->nz = (int)floorf((res->maxZ - res->minZ) / res->dz) + 1;

    return 1;
}

int generateBackgroundMesh(const ConfigFile* config, const Mesh* mesh)
{
    int result = 1;

    Resistivity res;
    if (!initResistivity(config, mesh, &res))  return 0;

    Node* nodes = NULL;
    size_t nNodes = 0;
    if (config->sourcesFile[0] != '\0'
        && !readXYZFile(config->sourcesFile, &nodes, &nNodes))
    {
        fprintf(stderr, "Failed to read sources/receivers file '%s'\n", config->sourcesFile);
        return 0;
    }

    float minSkinDepth = skinDepth(config->frequency, res.minResistivity);
    float skinSize = minSkinDepth / config->rSkinDepth;
    float sourceSize = fminimum(config->emitterLength / config->rsFactor, skinSize);

    BinGrid grid = { 0 };
    float sourceRadius = skinSize * 3.0f;
    if (nNodes > 0 && !buildBinGrid(&grid, nodes, nNodes, res.minX, res.maxX,
        res.minY, res.maxY, res.minZ, res.maxZ, sourceRadius))
    {
        fprintf(stderr, "Failed to build bin grid\n");
        result = 0;
        goto out_free_nodes;
    }

    FILE* file = fopen(config->backgroundMeshFile, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Could not create or open background mesh file '%s': %s\n",
            config->backgroundMeshFile, strerror(errno));
        result = 0;
        goto out_free_grid;
    }

    float step = skinSize;
    float radius = skinSize * 4.0f;
    fprintf(file, "View \"Background Mesh\" {\n");
    for (size_t n = 0; n < nNodes; ++n)
    {
        float x = nodes[n].x;
        float y = nodes[n].y;
        float z = nodes[n].z;

        float iMin = clampf(x - radius, res.minX, res.maxX);
        float iMax = clampf(x + radius + 1.0f, res.minX, res.maxX);
        for (float i = iMin; i <= iMax; i += step)
        {
            float jMin = clampf(y - radius, res.minY, res.maxY);
            float jMax = clampf(y + radius + 1.0f, res.minY, res.maxY);
            for (float j = jMin; j <= jMax; j += step)
            {
                float kMin = clampf(z - radius, res.minZ, res.maxZ);
                float kMax = clampf(z + radius + 1.0f, res.minZ, res.maxZ);
                for (float k = kMin; k <= kMax; k += step)
                {
                    float size = elementSize(&grid, nodes, config->growthFactor,
                        i, j, k, skinSize, sourceSize);
                    size *= config->elemSizeScale;
                    fprintf(file, "SP(%f,%f,%f){%f};\n", i, j, k, size);
                }
            }

        }
    }
    fprintf(file, "};\n");

    fclose(file);
out_free_grid:
    freeBinGrid(&grid);
out_free_nodes:
    free(nodes);
    return result;
}
