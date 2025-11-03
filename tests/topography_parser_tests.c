/*
    Filename: topography_parser_tests.c
    Author: David F. Meretzki
    Date: 2025-10-25

    Description:
    This file contains the tests for the topography parser
*/

#include <stdio.h>
#include <stdlib.h>

#include "topography_parser.h"
#include "utils.h"

static int testReadTopographyFile(char* projectRootDir)
{
    int result = 0;
    Topography topo = { 0 };
    char filename[256];
    combinePaths(filename, projectRootDir, "tests/test_topography");
    size_t expectedNx = 3;
    size_t expectedNy = 2;
    double expectedX[3] = { 7.1860000E+05, 7.1875302E+05, 7.1890604E+05 };
    double expectedY[2] = { 1.1526000E+06, 1.1526972E+06 };
    double expectedValues[6] = {
        1.0402600E+03, 1.0372841E+03,
        1.0543516E+03, 1.1016902E+03,
        1.1640649E+03, 1.2101630E+03
    };

    if (!readTopographyFile(filename, &topo))
    {
        printf("Failed to read topography file %s\n", filename);
        return 1;
    }
    if (topo.nx != expectedNx || topo.ny != expectedNy)
    {
        printf("Expected topo dimensions (%zu, %zu) but found (%zu, %zu)\n",
            expectedNx, expectedNy, topo.nx, topo.ny);
        result = 1;
        goto out_free_topo;
    }
    for (size_t i = 0; i < topo.nx; ++i)
    {
        if (topo.xGrid[i] != expectedX[i])
        {
            printf("X grid value %zu mismatch: expected %f but found %f\n",
                i, expectedX[i], topo.xGrid[i]);
            result = 1;
            goto out_free_topo;
        }
    }
    for (size_t i = 0; i < topo.ny; ++i)
    {
        if (topo.yGrid[i] != expectedY[i])
        {
            printf("Y grid value %zu mismatch: expected %f but found %f\n",
                i, expectedY[i], topo.yGrid[i]);
            result = 1;
            goto out_free_topo;
        }
    }
    for (size_t j = 0; j < topo.ny; ++j)
    {
        for (size_t i = 0; i < topo.nx; ++i)
        {
            size_t index = j * topo.nx + i;
            if (topo.values[index] != expectedValues[index])
            {
                printf("Topo value %zu mismatch: expected %f but found %f\n",
                    index, expectedValues[index], topo.values[index]);
                result = 1;
                goto out_free_topo;
            }
        }
    }

out_free_topo:
    freeTopography(&topo);
    return result;
}

static int testReadRawTopographyFile(char* projectRootDir)
{
    int result = 0;
    Node* nodes = NULL;
    size_t nNodes = 0;
    char filename[256];
    combinePaths(filename, projectRootDir, "tests/test_raw_topography");
    size_t expectedNNodes = 11;
    Node expectedNodes[] = {
        { 718600, 1.1755e+06, 400.007 },
        { 718800, 1.1748e+06, 399.646 },
        { 719000, 1.1698e+06, 419.834 },
        { 719200, 1.1698e+06, 440.044 },
        { 719400, 1.1696e+06, 379.719 },
        { 719600, 1.1696e+06, 379.473 },
        { 719800, 1.1692e+06, 379.816 },
        { 720000, 1.1692e+06, 359.234 },
        { 720200, 1.1695e+06, 439.547 },
        { 720400, 1.1695e+06, 480.115 },
        { 720600, 1.1732e+06, 479.897 },
    };

    if (!readRawTopographyFile(filename, &nodes, &nNodes))
    {
        printf("Failed to read raw topography file %s\n", filename);
        return 1;
    }
    if (nNodes != expectedNNodes)
    {
        printf("Expected %zu nodes but found %zu\n", expectedNNodes, nNodes);
        result = 1;
        goto out_free_nodes;
    }
    for (size_t i = 0; i < nNodes; ++i)
    {
        if (nodes[i].x != expectedNodes[i].x ||
            nodes[i].y != expectedNodes[i].y ||
            nodes[i].z != expectedNodes[i].z)
        {
            printf("Node %zu mismatch: expected (%f, %f, %f) but found (%f, %f, %f)\n",
                i,
                expectedNodes[i].x, expectedNodes[i].y, expectedNodes[i].z,
                nodes[i].x, nodes[i].y, nodes[i].z);
            result = 1;
            goto out_free_nodes;
        }
    }

out_free_nodes:
    free(nodes);
    return result;
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <project_root_directory>\n", argv[0]);
        return 1;
    }

    if (testReadTopographyFile(argv[1]) != 0) return 1;
    if (testReadRawTopographyFile(argv[1]) != 0) return 1;

    return 0;
}
