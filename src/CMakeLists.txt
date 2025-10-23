add_library(interpol_topography_lib
    config_file.c
    mesh.c
    msh_parser.c
    msh_tokenizer.c
    utils.c
)
target_include_directories(interpol_topography_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)