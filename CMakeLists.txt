cmake_minimum_required(VERSION 3.13)
project(capi VERSION "2.3.2")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake-modules")
set(CMAKE_CXX_FLAGS "-Ofast -Wall -Wextra")
set(PY_MODULE_TARGET _vis_graph)
set(TEST_TARGET "${PROJECT_NAME}_test")
set(EXAMPLE_TARGET "${PROJECT_NAME}_example")
set(CPP_LIBS_DIR ${CMAKE_SOURCE_DIR}/capi/cpp_libs)

# Include CMake modules
include(FetchContent)

# Git
find_package(Git REQUIRED)

# OpenMP
find_package(OpenMP REQUIRED)

# TBB
find_package(TBB REQUIRED)

# S2
find_package(SWIG 4.0 COMPONENTS python)
FetchContent_Declare(
    s2
    GIT_REPOSITORY https://github.com/google/s2geometry.git
    GIT_TAG v0.9.0
    BUILD_COMMAND make
)

FetchContent_MakeAvailable(s2)

# Currently, Scikit-build does not support FindPython, so we convert the
# provided hints ourselves.
if(SKBUILD)
    set(Python_EXECUTABLE "${PYTHON_EXECUTABLE}")
    set(Python_INCLUDE_DIR "${PYTHON_INCLUDE_DIR}")
    set(Python_LIBRARY "${PYTHON_LIBRARY}")
    set(DUMMY "${PYTHON_VERSION_STRING}")  # Not needed, silences a warning
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Scikit-Build does not add your site-packages to the search path automatically,
# so we need to add it _or_ the pybind11 specific directory here.
execute_process(
        COMMAND
        "${Python_EXECUTABLE}" -c
        "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE _tmp_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE)
list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

# Now we can find pybind11
find_package(pybind11 CONFIG REQUIRED)

# Get our source files
set(VIS_GRAPH_IMPL_DIR ${CMAKE_SOURCE_DIR}/capi/src/implementation/visibility_graphs)

file(GLOB_RECURSE VIS_GRAPH_IMPLEMENTATION_FILES "${VIS_GRAPH_IMPL_DIR}/code/*.cpp" "${VIS_GRAPH_IMPL_DIR}/code/*.hpp")
include_directories(${VIS_GRAPH_IMPL_DIR}/code)

# Get fmt library
include_directories(${CPP_LIBS_DIR}/fmt/include)
file(GLOB_RECURSE FMT_SOURCE "${CPP_LIBS_DIR}/fmt/*.cc" "${CPP_LIBS_DIR}/fmt/*.h")

# Get mio library
include_directories(${CPP_LIBS_DIR}/mio)
file(GLOB_RECURSE MIO_SOURCE "${CPP_LIBS_DIR}/mio/*.hpp" "${CPP_LIBS_DIR}/mio/*.cpp")

# Create python module
pybind11_add_module(${PY_MODULE_TARGET} MODULE
        ${VIS_GRAPH_IMPLEMENTATION_FILES}
        ${FMT_SOURCE}
        ${MIO_SOURCE}
        ${VIS_GRAPH_IMPL_DIR}/bindings.cpp)
target_compile_definitions(${PY_MODULE_TARGET} PRIVATE VERSION_INFO=${PROJECT_VERSION})
target_link_libraries(${PY_MODULE_TARGET} PRIVATE
        OpenMP::OpenMP_CXX
        s2
        tbb)
install(TARGETS ${PY_MODULE_TARGET} DESTINATION .)

# Setup unit tests
set(TEST_DIR "capi/test")
file(GLOB_RECURSE TEST_FILES "${TEST_DIR}/*.cpp" "${TEST_DIR}/*.hpp")

# Generate a test executable
include_directories(${CPP_LIBS_DIR}/catch2)
file(GLOB_RECURSE CATCH_SOURCE "${CPP_LIBS_DIR}/catch2/*.cpp" "${CPP_LIBS_DIR}/catch2/*.hpp")

add_executable(${TEST_TARGET}
        ${TEST_FILES}
        ${VIS_GRAPH_IMPLEMENTATION_FILES}
        ${CATCH_SOURCE}
        ${MIO_SOURCE}
        ${FMT_SOURCE})

target_link_libraries(${TEST_TARGET} PRIVATE
        OpenMP::OpenMP_CXX
        s2
        tbb)

add_executable(${EXAMPLE_TARGET}
        ${VIS_GRAPH_IMPLEMENTATION_FILES}
        ${VIS_GRAPH_IMPL_DIR}/example.cpp
        ${MIO_SOURCE}
        ${FMT_SOURCE})

target_link_libraries(${EXAMPLE_TARGET} PRIVATE
        OpenMP::OpenMP_CXX
        s2
        tbb)
