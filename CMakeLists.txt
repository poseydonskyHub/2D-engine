cmake_minimum_required(VERSION 3.20)

project(Tiny2D LANGUAGES CXX)

add_executable(Tiny2D WIN32
    src/main.cpp
    src/Engine.cpp
    src/Game.cpp
)

target_compile_features(Tiny2D PRIVATE cxx_std_20)

if (MSVC)
    target_compile_options(Tiny2D PRIVATE /W4 /permissive-)
else()
    target_compile_options(Tiny2D PRIVATE -Wall -Wextra -pedantic)
endif()

