cmake_minimum_required(VERSION 3.8.0)

project(DynamicArray CXX)

set(SOURCES
    DynamicArray.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})