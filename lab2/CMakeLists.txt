cmake_minimum_required(VERSION 3.8.0)

project(DynamicArray CXX)

set(SOURCES
    DynamicArray.cpp
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cppunit/lib)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} cppunit)
target_include_directories(${PROJECT_NAME}
    PRIVATE ../third_party/cppunit/include
)