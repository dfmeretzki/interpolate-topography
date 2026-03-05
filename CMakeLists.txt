cmake_minimum_required(VERSION 3.25)
project(interpol_topography LANGUAGES C)

# GNU Scientific Library
find_package(GSL REQUIRED)

# segyio library
set(BUILD_BIN OFF CACHE BOOL "Build segyio applications" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build language bindings shared" FORCE)
set(BUILD_PYTHON OFF CACHE BOOL "Build python bindings" FORCE)
set(BUILD_MEX OFF CACHE BOOL "Build matlab bindings" FORCE)
add_subdirectory(libs/segyio)

option(TESTING "Enable testing" ON)

if(TESTING)
	enable_testing()
endif()

add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE c_std_23)
target_compile_options(compiler_flags INTERFACE
    -Wall -Wextra -pedantic -Werror
)

add_executable(interpol_topography src/main.c)


add_subdirectory(src)

target_include_directories(interpol_topography PUBLIC include)

target_link_libraries(interpol_topography PUBLIC
    compiler_flags
    interpol_topography_lib
)

if(TESTING)
	add_subdirectory(tests)
endif()