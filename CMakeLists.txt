cmake_minimum_required(VERSION 3.25)
project(interpol_topography LANGUAGES C)

set(SOURCE_FILES
    src/main.c
)

add_executable(interpol_topography ${SOURCE_FILES})

target_include_directories(interpol_topography PUBLIC include)
