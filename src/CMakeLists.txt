add_library(interpol_topography_lib
    config_file.c
    msh_tokenizer.c
    utils.c
)
target_include_directories(interpol_topography_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)