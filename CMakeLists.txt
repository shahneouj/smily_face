cmake_minimum_required(VERSION 3.16)

project(framebuffer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

add_executable(framebuffer
    main.c
)

target_link_libraries(framebuffer PRIVATE
    SDL3::SDL3
    m
)

add_custom_target(run
    COMMAND framebuffer
    DEPENDS framebuffer
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
