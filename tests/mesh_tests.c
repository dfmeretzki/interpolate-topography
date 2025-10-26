/*
    Filename: mesh_tests.c
    Author: David F. Meretzki
    Date: 2025-10-26

    Description:
    This file contains the tests for the mesh functions
*/

#include <math.h>
#include <stdio.h>

#include "mesh.h"
#include "msh_parser.h"
#include "topography_parser.h"

static int meshTests()
{
    {
        // Test interpolate topography
        Mesh mesh = { 0 };
        Mesh resultMesh = { 0 };
        Topography topo = { 0 };
        // Working directory is interpol_topo/out/build/debug/tests when running tests
        char* meshFile = "../../../../tests/test_skin.msh";
        char* resultMeshFile = "../../../../tests/test_skin_topo.msh";
        char* topoFile = "../../../../tests/test_skin_topography";
        if (!readMshFile(meshFile, &mesh))
        {
            printf("Failed to read MSH file %s\n", meshFile);
            return 1;
        }
        if (!readMshFile(resultMeshFile, &resultMesh))
        {
            printf("Failed to read MSH file %s\n", resultMeshFile);
            freeMesh(&mesh);
            return 1;
        }
        if (!readTopographyFile(topoFile, &topo))
        {
            printf("Failed to read topography file %s\n", topoFile);
            freeMesh(&mesh);
            freeMesh(&resultMesh);
            return 1;
        }

        ConfigFile config = { 0 };
        config.surfaceMeshFaces[0] = 6; // face region to apply topography
        if (!interpolateTopography(&config, &topo, &mesh))
        {
            printf("Failed to interpolate topography\n");
            freeMesh(&mesh);
            freeMesh(&resultMesh);
            freeTopography(&topo);
            return 1;
        }

        for (size_t i = 0; i < mesh.nNodes; ++i)
        {
            // Compare z-coordinates with a tolerance since msh files
            // may have slight differences due to floating point representation
            if (fabs(mesh.nodes[i].z - resultMesh.nodes[i].z) > 1e-3)
            {
                printf("Node %zu z-coordinate mismatch: expected %lf but found %lf\n",
                    i + 1, resultMesh.nodes[i].z, mesh.nodes[i].z);
                return 1;
            }
        }

        freeMesh(&mesh);
        freeMesh(&resultMesh);
        freeTopography(&topo);
    }
    return 0;
}

int main()
{
    return meshTests();
}
