/*
    Filename: msh_parser_tests.c
    Author: David F. Meretzki
    Date: 2025-10-23

    Description:
    This file contains the tests for the msh parser
*/

#include <stdio.h>

#include "msh_parser.h"

static int parserTests()
{

    {
        // Test reading a simple MSH v1 file
        Mesh mesh = { 0 };
        // Working directory is interpol_topo/out/build/debug/tests when running tests
        char* filename = "../../../../tests/test.msh";
        double expectedNodes[3][3] = {
            {718600.0, 1152600.0, -6000.0},
            {741400.0, 1152600.0, -6000.0},
            {741400.0, 1170000.0, -6000.0}
        };
        int expectedElems[4][8] = {
            {1, 15, 0, 1, 1, 0, -1, -1},
            {2, 1, 0, 1, 2, 0, 1, -1},
            {3, 2, 0, 1, 3, 0, 1, 2},
            {4, 2, 0, 1, 3, 0, 1, 2},
        };

        if (!readMshFile(filename, &mesh))
        {
            printf("Failed to read MSH file %s\n", filename);
            freeMesh(&mesh);
            return 1;
        }
        if (mesh.nNodes != 3)
        {
            printf("Expected 3 nodes but found %zu\n", mesh.nNodes);
            freeMesh(&mesh);
            return 1;
        }
        for (int i = 0; i < 3; ++i)
        {
            if (mesh.nodeIndex[i] != i)
            {
                printf("Node index %d mismatch: expected %d but found %zu\n",
                    i + 1, i, mesh.nodeIndex[i]);
                freeMesh(&mesh);
                return 1;
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
                freeMesh(&mesh);
                return 1;
            }
        }
        if (mesh.nElems != 4)
        {
            printf("Expected 4 elements but found %zu\n", mesh.nElems);
            freeMesh(&mesh);
            return 1;
        }
        for (int i = 0; i < 4; ++i)
        {
            if (mesh.elements[i].type != expectedElems[i][1])
            {
                printf("Element %d type mismatch: expected %d but found %d\n",
                    i + 1, expectedElems[i][1], mesh.elements[i].type);
                freeMesh(&mesh);
                return 1;
            }
            if (mesh.elements[i].regPhys != expectedElems[i][2])
            {
                printf("Element %d regPhys mismatch: expected %d but found %d\n",
                    i + 1, expectedElems[i][2], mesh.elements[i].regPhys);
                freeMesh(&mesh);
                return 1;
            }
            if (mesh.elements[i].regElem != expectedElems[i][3])
            {
                printf("Element %d regElem mismatch: expected %d but found %d\n",
                    i + 1, expectedElems[i][3], mesh.elements[i].regElem);
                freeMesh(&mesh);
                return 1;
            }
            if (mesh.elements[i].nNodes != expectedElems[i][4])
            {
                printf("Element %d nNodes mismatch: expected %d but found %zu\n",
                    i + 1, expectedElems[i][4], mesh.elements[i].nNodes);
                freeMesh(&mesh);
                return 1;
            }
            for (int j = 0; j < mesh.elements[i].nNodes; ++j)
            {
                if (mesh.elements[i].nodes[j] != (size_t)expectedElems[i][5 + j])
                {
                    printf("Element %d node %d mismatch: expected %d but found %zu\n",
                        i + 1, j + 1, expectedElems[i][5 + j], mesh.elements[i].nodes[j]);
                    freeMesh(&mesh);
                    return 1;
                }
            }
        }
        freeMesh(&mesh);
    }

    return 0;
}

int main()
{
    return parserTests();
}
