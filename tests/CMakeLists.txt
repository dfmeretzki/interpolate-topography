add_executable(msh_tokenizer_tests msh_tokenizer_tests.c)
target_include_directories(msh_tokenizer_tests
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(msh_tokenizer_tests PUBLIC
	compiler_flags
	interpol_topography_lib
)
add_test(msh_tokenizer_tests msh_tokenizer_tests)

add_executable(msh_parser_tests msh_parser_tests.c)
target_include_directories(msh_parser_tests
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(msh_parser_tests PUBLIC
	compiler_flags
	interpol_topography_lib
)
add_test(msh_parser_tests msh_parser_tests)

add_executable(topography_parser_tests topography_parser_tests.c)
target_include_directories(topography_parser_tests
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(topography_parser_tests PUBLIC
	compiler_flags
	interpol_topography_lib
)
add_test(topography_parser_tests topography_parser_tests)

add_executable(mesh_tests mesh_tests.c)
target_include_directories(mesh_tests
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(mesh_tests PUBLIC
	compiler_flags
	interpol_topography_lib
	"$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:m>"
)
add_test(mesh_tests mesh_tests)