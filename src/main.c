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

    // Interpolate the topography onto the mesh
    if (!interpolate(&config, &mesh))
    {
        fprintf(stderr, "Failed to interpolate topography onto the mesh\n");
        result = EXIT_FAILURE;
        goto out_free_mesh;
    }

    // Smooth the mesh faces
    if (!smoothMesh(&config, &mesh))
    {
        fprintf(stderr, "Failed to smooth the mesh faces\n");
        result = EXIT_FAILURE;
        goto out_free_mesh;
    }

    // Write the mesh to a .msh file
    if (!writeMshFile(config.skinMeshFileOut, &mesh, MSH_V1))
    {
        fprintf(stderr, "Failed to write the resulting .msh file '%s'\n",
            config.skinMeshFileOut);
        result = EXIT_FAILURE;
        goto out_free_mesh;
    }

out_free_mesh:
    freeMesh(&mesh);
    return result;
}
