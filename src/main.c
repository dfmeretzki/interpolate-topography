#include <stdio.h>
#include <stdlib.h>

#include "config_file.h"
#include "msh_parser.h"

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

    freeMesh(&mesh);

    return EXIT_SUCCESS;
}
