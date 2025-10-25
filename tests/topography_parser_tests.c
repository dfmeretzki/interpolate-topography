/*
    Filename: topography_parser_tests.c
    Author: David F. Meretzki
    Date: 2025-10-25

    Description:
    This file contains the tests for the topography parser
*/

#include <stdio.h>

#include "topography_parser.h"

static int topographyParserTests()
{
    {
        // Test reading a simple topography file
        Topography topo;
        // Working directory is interpol_topo/out/build/debug/tests when running tests
        char* filename = "../../../../tests/test_topography";
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
            return 1;
        }
        for (size_t i = 0; i < topo.nx; ++i)
        {
            if (topo.xGrid[i] != expectedX[i])
            {
                printf("X grid value %zu mismatch: expected %f but found %f\n",
                    i, expectedX[i], topo.xGrid[i]);
                return 1;
            }
        }
        for (size_t i = 0; i < topo.ny; ++i)
        {
            if (topo.yGrid[i] != expectedY[i])
            {
                printf("Y grid value %zu mismatch: expected %f but found %f\n",
                    i, expectedY[i], topo.yGrid[i]);
                return 1;
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
                    return 1;
                }
            }
        }
        freeTopography(&topo);
    }

    return 0;
}

int main()
{
    return topographyParserTests();
}
