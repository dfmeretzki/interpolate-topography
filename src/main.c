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

    int result = EXIT_SUCCESS;

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
    if (!readTopographyFile(config.topoFiles[0], &topo))
    {
        fprintf(stderr, "Failed to parse topography file '%s'\n", config.topoFiles[0]);
        result = EXIT_FAILURE;
        goto out_free_mesh;
    }

    // Interpolate the topography onto the mesh
    if (!interpolateTopography(&config, &topo, &mesh))
    {
        fprintf(stderr, "Failed to interpolate topography onto the mesh\n");
        result = EXIT_FAILURE;
        goto out_free_topo;
    }

    // Smooth the mesh faces
    if (!smoothMesh(&config, &mesh))
    {
        fprintf(stderr, "Failed to smooth the mesh faces\n");
        result = EXIT_FAILURE;
        goto out_free_topo;
    }

    // Write the mesh to a .msh file
    if (!writeMshFile(config.skinMeshFileOut, &mesh, MSH_V1))
    {
        fprintf(stderr, "Failed to write the resulting .msh file '%s'\n",
            config.skinMeshFileOut);
        result = EXIT_FAILURE;
        goto out_free_topo;
    }

out_free_topo:
    freeTopography(&topo);
out_free_mesh:
    freeMesh(&mesh);
    return result;
}
