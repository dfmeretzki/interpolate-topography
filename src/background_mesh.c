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
#include <string.h>

#include "resistivity_parser.h"
#include "resistivity.h"

int generateBackgroundMesh(const ConfigFile* config)
{
    Resistivity res;
    if (!readSEGYFile(config->resistivityFile, &res))
    {
        fprintf(stderr, "Failed to read resistivity file '%s'\n", config->resistivityFile);
        return 0;
    }

    float minSkinDepth = skinDepth(config->frequency, res.minResistivity);
    float skinSize = minSkinDepth / config->rSkinDepth;
    // float sourceSize = fminimum(config->emitterLength / config->rsFactor, skinSize);

    FILE* file = fopen(config->backgroundMeshFile, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Could not create or open background mesh file '%s': %s\n",
            config->backgroundMeshFile, strerror(errno));
        return 0;
    }

    fprintf(file, "View \"Background Mesh\" {\n");
    for (int i = 0; i <= res.nx; ++i)
    {
        float x = res.minX + i * res.dx;
        for (int j = 0; j <= res.ny; ++j)
        {
            float y = res.minY + j * res.dy;
            for (int k = 0; k <= res.nz; ++k)
            {
                float z = res.minZ + k * res.dz;
                fprintf(file, "SP(%f,%f,%f){%f};\n", x, y, z, skinSize);
            }
        }

    }
    fprintf(file, "};\n");

    fclose(file);
    return 1;
}
