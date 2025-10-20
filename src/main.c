#include <stdio.h>
#include <stdlib.h>

#include "config_file.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: interpol_topography <config file>\n");
        exit(EXIT_FAILURE);
    }

    ConfigFile config = { 0 };
    readConfigFile(argv[1], &config);

    return EXIT_SUCCESS;
}
