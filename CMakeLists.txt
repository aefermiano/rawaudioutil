cmake_minimum_required(VERSION 3.5)

project(rawaudioutil LANGUAGES C)

set(CMAKE_C_FLAGS "-O3")

add_executable(rawaudioutil main.c)

install(TARGETS rawaudioutil)
