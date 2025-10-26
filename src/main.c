#include <stdio.h>
#include <stdlib.h>

#include "config_file.h"
#include "msh_parser.h"
#include "topography_parser.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: interpol_topography <config file>\n");
        exit(EXIT_FAILURE);
    }

    // Read the configuration file
    ConfigFile config = { 0 };
    readConfigFile(argv[1], &config);

    // Parse the .msh file
    Mesh mesh = { 0 };
    if (!readMshFile(config.skinMeshFileIn, &mesh))
    {
        fprintf(stderr, "Failed to parse .msh file '%s'\n", config.skinMeshFileIn);
        exit(EXIT_FAILURE);
    }

    // Parse the topography file
    Topography topo = { 0 };
    if (!readTopographyFile(config.topoFile, &topo))
    {
        fprintf(stderr, "Failed to parse topography file '%s'\n", config.topoFile);
        freeMesh(&mesh);
        exit(EXIT_FAILURE);
    }

    // Interpolate the topography onto the mesh
    if (!interpolateTopography(&config, &topo, &mesh))
    {
        fprintf(stderr, "Failed to interpolate topography onto the mesh\n");
        freeTopography(&topo);
        freeMesh(&mesh);
        exit(EXIT_FAILURE);
    }

    freeTopography(&topo);
    freeMesh(&mesh);

    return EXIT_SUCCESS;
}
