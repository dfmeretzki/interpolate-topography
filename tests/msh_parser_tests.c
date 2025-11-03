/*
    Filename: msh_parser_tests.c
    Author: David F. Meretzki
    Date: 2025-10-23

    Description:
    This file contains the tests for the msh parser
*/

#include <math.h>
#include <stdio.h>

#include "msh_parser.h"
#include "utils.h"

static int testReadMshFileV1(char* projectRootDir)
{
    int result = 0;
    Mesh mesh = { 0 };
    char filename[256];
    combinePaths(filename, projectRootDir, "tests/test.msh");
    double expectedNodes[3][3] = {
        {718600.0, 1152600.0, -6000.0},
        {741400.0, 1152600.0, -6000.0},
        {741400.0, 1170000.0, -6000.0}
    };
    size_t expectedElems[4][8] = {
        {1, 15, 0, 1, 1, 0, -1, -1},
        {2, 1, 0, 1, 2, 0, 1, -1},
        {3, 2, 0, 1, 3, 0, 1, 2},
        {4, 2, 0, 1, 3, 0, 1, 2},
    };

    if (!readMshFile(filename, &mesh))
    {
        printf("Failed to read MSH file %s\n", filename);
        return 1;
    }
    if (mesh.nNodes != 3)
    {
        printf("Expected 3 nodes but found %zu\n", mesh.nNodes);
        result = 1;
        goto out_free_mesh;
    }
    for (size_t i = 0; i < 3; ++i)
    {
        if (mesh.nodeIndex[i] != i)
        {
            printf("Node index %zu mismatch: expected %zu but found %zu\n",
                i + 1, i, mesh.nodeIndex[i]);
            result = 1;
            goto out_free_mesh;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        if (mesh.nodes[i].x != expectedNodes[i][0]
            || mesh.nodes[i].y != expectedNodes[i][1]
            || mesh.nodes[i].z != expectedNodes[i][2])
        {
            printf("Node %d coordinates mismatch: expected (%.1f, %.1f, %.1f) but found (%.1f, %.1f, %.1f)\n",
                i + 1, expectedNodes[i][0], expectedNodes[i][1], expectedNodes[i][2],
                mesh.nodes[i].x, mesh.nodes[i].y, mesh.nodes[i].z);
            result = 1;
            goto out_free_mesh;
        }
    }
    if (mesh.nElems != 4)
    {
        printf("Expected 4 elements but found %zu\n", mesh.nElems);
        result = 1;
        goto out_free_mesh;
    }
    for (size_t i = 0; i < 4; ++i)
    {
        if (mesh.elements[i].type != expectedElems[i][1])
        {
            printf("Element %zu type mismatch: expected %zu but found %u\n",
                i + 1, expectedElems[i][1], mesh.elements[i].type);
            result = 1;
            goto out_free_mesh;
        }
        if (mesh.elements[i].regPhys != expectedElems[i][2])
        {
            printf("Element %zu regPhys mismatch: expected %zu but found %u\n",
                i + 1, expectedElems[i][2], mesh.elements[i].regPhys);
            result = 1;
            goto out_free_mesh;
        }
        if (mesh.elements[i].regElem != expectedElems[i][3])
        {
            printf("Element %zu regElem mismatch: expected %zu but found %u\n",
                i + 1, expectedElems[i][3], mesh.elements[i].regElem);
            result = 1;
            goto out_free_mesh;
        }
        if (mesh.elements[i].nNodes != expectedElems[i][4])
        {
            printf("Element %zu nNodes mismatch: expected %zu but found %zu\n",
                i + 1, expectedElems[i][4], mesh.elements[i].nNodes);
            result = 1;
            goto out_free_mesh;
        }
        for (size_t j = 0; j < mesh.elements[i].nNodes; ++j)
        {
            if (mesh.elements[i].nodes[j] != (size_t)expectedElems[i][5 + j])
            {
                printf("Element %zu node %zu mismatch: expected %zu but found %zu\n",
                    i + 1, j + 1, expectedElems[i][5 + j], mesh.elements[i].nodes[j]);
                result = 1;
                goto out_free_mesh;
            }
        }
    }

out_free_mesh:
    freeMesh(&mesh);
    return result;
}

static int testWriteMshFileV1(char* projectRootDir)
{
    int result = 0;
    Mesh mesh = { 0 };
    Mesh resultMesh = { 0 };
    char writeFile[256];
    combinePaths(writeFile, projectRootDir, "tests/test_skin_modified.msh");
    char resultMeshFile[256];
    combinePaths(resultMeshFile, projectRootDir, "tests/test_skin.msh");
    if (!readMshFile(resultMeshFile, &resultMesh))
    {
        printf("Failed to read MSH file %s\n", resultMeshFile);
        return 1;
    }
    if (!writeMshFile(writeFile, &resultMesh, MSH_V1))
    {
        printf("Failed to write MSH file %s\n", writeFile);
        result = 1;
        goto out_free_result_mesh;
    }
    if (!readMshFile(writeFile, &mesh))
    {
        printf("Failed to read MSH file %s\n", writeFile);
        result = 1;
        goto out_free_result_mesh;
    }
    if (mesh.nNodes != resultMesh.nNodes)
    {
        printf("Number of nodes mismatch: expected %zu but found %zu\n",
            resultMesh.nNodes, mesh.nNodes);
        result = 1;
        goto out_free_mesh;
    }
    for (size_t i = 0; i < mesh.nNodes; ++i)
    {
        // Compare z-coordinates with a tolerance since msh files
        // may have slight differences due to floating point representation
        if (fabs(mesh.nodes[i].x - resultMesh.nodes[i].x) > 1e-6)
        {
            printf("Node %zu x-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].x, mesh.nodes[i].x);
            result = 1;
            goto out_free_mesh;
        }
        if (fabs(mesh.nodes[i].y - resultMesh.nodes[i].y) > 1e-6)
        {
            printf("Node %zu y-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].y, mesh.nodes[i].y);
            result = 1;
            goto out_free_mesh;
        }
        if (fabs(mesh.nodes[i].z - resultMesh.nodes[i].z) > 1e-6)
        {
            printf("Node %zu z-coordinate mismatch: expected %lf but found %lf\n",
                i + 1, resultMesh.nodes[i].z, mesh.nodes[i].z);
            result = 1;
            goto out_free_mesh;
        }
    }
    if (mesh.nElems != resultMesh.nElems)
    {
        printf("Number of elements mismatch: expected %zu but found %zu\n",
            resultMesh.nElems, mesh.nElems);
        result = 1;
        goto out_free_mesh;
    }
    for (size_t i = 0; i < mesh.nElems; ++i)
    {
        if (mesh.elements[i].type != resultMesh.elements[i].type)
        {
            printf("Element %zu type mismatch: expected %u but found %u\n",
                i + 1, resultMesh.elements[i].type, mesh.elements[i].type);
            result = 1;
            goto out_free_mesh;
        }
        if (mesh.elements[i].regPhys != resultMesh.elements[i].regPhys)
        {
            printf("Element %zu regPhys mismatch: expected %u but found %u\n",
                i + 1, resultMesh.elements[i].regPhys, mesh.elements[i].regPhys);
            result = 1;
            goto out_free_mesh;
        }
        if (mesh.elements[i].regElem != resultMesh.elements[i].regElem)
        {
            printf("Element %zu regElem mismatch: expected %u but found %u\n",
                i + 1, resultMesh.elements[i].regElem, mesh.elements[i].regElem);
            result = 1;
            goto out_free_mesh;
        }
        if (mesh.elements[i].nNodes != resultMesh.elements[i].nNodes)
        {
            printf("Element %zu nNodes mismatch: expected %zu but found %zu\n",
                i + 1, resultMesh.elements[i].nNodes, mesh.elements[i].nNodes);
            result = 1;
            goto out_free_mesh;
        }
        for (size_t j = 0; j < mesh.elements[i].nNodes; ++j)
        {
            if (mesh.elements[i].nodes[j] != resultMesh.elements[i].nodes[j])
            {
                printf("Element %zu node %zu mismatch: expected %zu but found %zu\n",
                    i + 1, j + 1, resultMesh.elements[i].nodes[j], mesh.elements[i].nodes[j]);
                result = 1;
                goto out_free_mesh;
            }
        }
    }

out_free_mesh:
    freeMesh(&mesh);
out_free_result_mesh:
    freeMesh(&resultMesh);
    return result;
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <project_root_directory>\n", argv[0]);
        return 1;
    }

    if (testReadMshFileV1(argv[1]) != 0) return 1;
    if (testWriteMshFileV1(argv[1]) != 0) return 1;

    return 0;
}
