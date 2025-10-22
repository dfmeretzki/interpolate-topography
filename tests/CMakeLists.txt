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
