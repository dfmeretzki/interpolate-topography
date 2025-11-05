/*
    Filename: topography_tests.c
    Author: David F. Meretzki
    Date: 2025-11-02

    Description:
    This file contains the tests for the topography functions
*/

#include <math.h>
#include <stdio.h>

#include "topography_parser.h"
#include "topography.h"
#include "utils.h"

static int testIncreaseTopographyResolution(char* projectRootDir)
{
    int result = 0;
    Topography topo = { 0 };
    Topography resultTopo = { 0 };
    char topoFile[256];
    combinePaths(topoFile, projectRootDir, "tests/test_skin_topography_raw");
    char resultTopoFile[256];
    combinePaths(resultTopoFile, projectRootDir, "tests/test_skin_topography");

    ConfigFile config = { 0 };
    config.nx = 150;
    config.ny = 180;
    if (!increaseTopographyResolution(&config, topoFile, &topo))
    {
        printf("Failed to increase topography resolution for file %s\n", topoFile);
        return 1;
    }

    if (!readTopographyFile(resultTopoFile, &resultTopo))
    {
        printf("Failed to read topography file %s\n", resultTopoFile);
        result = 1;
        goto out_free_topo;
    }

    if (topo.nx != resultTopo.nx || topo.ny != resultTopo.ny)
    {
        printf("Topography resolution mismatch: expected (%zu, %zu) but found (%zu, %zu)\n",
            resultTopo.nx, resultTopo.ny, topo.nx, topo.ny);
        result = 1;
        goto out_free_result_topo;
    }
    for (size_t i = 0; i < topo.nx; ++i)
    {
        // Compare xGrid with a tolerance since topography files may have
        // slight differences due to floating point representation
        if (fabs(topo.xGrid[i] - resultTopo.xGrid[i]) > 0.1)
        {
            printf("Topography xGrid mismatch at index %zu: expected %lf but found %lf\n",
                i, resultTopo.xGrid[i], topo.xGrid[i]);
            result = 1;
            goto out_free_result_topo;
        }
    }
    for (size_t i = 0; i < topo.ny; ++i)
    {
        // Compare yGrid with a tolerance since topography files may have
        // slight differences due to floating point representation
        if (fabs(topo.yGrid[i] - resultTopo.yGrid[i]) > 0.1)
        {
            printf("Topography yGrid mismatch at index %zu: expected %lf but found %lf\n",
                i, resultTopo.yGrid[i], topo.yGrid[i]);
            result = 1;
            goto out_free_result_topo;
        }
    }
    for (size_t i = 0; i < topo.nx * topo.ny; ++i)
    {
        // Compare values with a error tolerance of 5% since a different 
        // interpolation method was used to generate the result file
        if (fabs(topo.values[i] - resultTopo.values[i]) > fabs(topo.values[i] * 0.05))
        {
            printf("Topography value mismatch at index %zu: expected %lf but found %lf\n",
                i, resultTopo.values[i], topo.values[i]);
            result = 1;
            goto out_free_result_topo;
        }
    }

out_free_result_topo:
    freeTopography(&resultTopo);
out_free_topo:
    freeTopography(&topo);
    return result;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <project_root_directory>\n", argv[0]);
        return 1;
    }
    return testIncreaseTopographyResolution(argv[1]);
}
