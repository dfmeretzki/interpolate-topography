add_library(interpol_topography_lib
    config_file.c
    mesh.c
    msh_parser.c
    msh_tokenizer.c
    topography_parser.c
    utils.c
)
target_include_directories(interpol_topography_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(interpol_topography_lib PUBLIC
    compiler_flags
    "$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:m>"
)