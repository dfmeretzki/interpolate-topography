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
#include "utils.h"

static int testInterpolateTopography(char* projectRootDir)
{
    int result = 0;
    Mesh mesh = { 0 };
    Mesh resultMesh = { 0 };
    Topography topo = { 0 };
    char meshFile[256];
    combinePaths(meshFile, projectRootDir, "tests/test_skin.msh");
    char resultMeshFile[256];
    combinePaths(resultMeshFile, projectRootDir, "tests/test_skin_topo.msh");
    char topoFile[256];
    combinePaths(topoFile, projectRootDir, "tests/test_skin_topography");

    if (!readMshFile(meshFile, &mesh))
    {
        printf("Failed to read MSH file %s\n", meshFile);
        return 1;
    }
    if (!readMshFile(resultMeshFile, &resultMesh))
    {
        printf("Failed to read MSH file %s\n", resultMeshFile);
        result = 1;
        goto out_free_mesh;
    }
    if (!readTopographyFile(topoFile, &topo))
    {
        printf("Failed to read topography file %s\n", topoFile);
        result = 1;
        goto out_free_result_mesh;
    }

    ConfigFile config = { 0 };
    config.surfaceMeshFaces[0] = 6; // face region to apply topography
    if (!interpolateTopography(&config, &topo, &mesh))
    {
        printf("Failed to interpolate topography\n");
        result = 1;
        goto out_free_topo;
    }

    for (size_t i = 0; i < mesh.nNodes; ++i)
    {
        // Compare z-coordinates with a tolerance since msh files
        // may have slight differences due to floating point representation
        if (fabs(mesh.nodes[i].z - resultMesh.nodes[i].z) > 1e-3)
        {
            printf("Node %zu z-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].z, mesh.nodes[i].z);
            result = 1;
            goto out_free_topo;
        }
    }

out_free_topo:
    freeTopography(&topo);
out_free_result_mesh:
    freeMesh(&resultMesh);
out_free_mesh:
    freeMesh(&mesh);
    return result;
}

static int testSmoothMesh(char* projectRootDir)
{
    int result = 0;
    Mesh mesh = { 0 };
    Mesh resultMesh = { 0 };
    char meshFile[256];
    combinePaths(meshFile, projectRootDir, "tests/test_skin_topo.msh");
    char resultMeshFile[256];
    combinePaths(resultMeshFile, projectRootDir, "tests/test_skin_topo_smooth.msh");
    if (!readMshFile(meshFile, &mesh))
    {
        printf("Failed to read MSH file %s\n", meshFile);
        return 1;
    }
    if (!readMshFile(resultMeshFile, &resultMesh))
    {
        printf("Failed to read MSH file %s\n", resultMeshFile);
        result = 1;
        goto out_free_mesh;
    }

    ConfigFile config = { 0 };
    config.meshFacesToSmooth[0] = 1;
    config.meshFacesToSmooth[1] = 2;
    config.meshFacesToSmooth[2] = 3;
    config.meshFacesToSmooth[3] = 4;
    config.meshFacesToSmooth[4] = 5;
    if (!smoothMesh(&config, &mesh))
    {
        printf("Failed to smooth the mesh\n");
        result = 1;
        goto out_free_result_mesh;
    }

    for (size_t i = 0; i < mesh.nNodes; ++i)
    {
        // Compare coordinates with a tolerance since msh files
        // may have slight differences due to floating point representation
        if (fabs(mesh.nodes[i].x - resultMesh.nodes[i].x) > 1)
        {
            printf("Node %zu x-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].x, mesh.nodes[i].x);
            result = 1;
            goto out_free_result_mesh;
        }
        if (fabs(mesh.nodes[i].y - resultMesh.nodes[i].y) > 1)
        {
            printf("Node %zu y-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].y, mesh.nodes[i].y);
            result = 1;
            goto out_free_result_mesh;
        }
        if (fabs(mesh.nodes[i].z - resultMesh.nodes[i].z) > 1)
        {
            printf("Node %zu z-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].z, mesh.nodes[i].z);
            result = 1;
            goto out_free_result_mesh;
        }
    }

out_free_result_mesh:
    freeMesh(&resultMesh);
out_free_mesh:
    freeMesh(&mesh);
    return result;
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <project_root_directory>\n", argv[0]);
        return 1;
    }

    if (testInterpolateTopography(argv[1]) != 0) return 1;
    if (testSmoothMesh(argv[1]) != 0) return 1;

    return 0;
}
