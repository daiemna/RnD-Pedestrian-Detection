cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(fh_image_lite)

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE IMAGE_LIB_SOURCES "src/image_lib/*.cpp")
# file(GLOB_RECURSE IMAGE_LIB_HEADERS "include/image_lib/")

set (INCLUDE_DIR "")
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIR ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIR)

FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

FIND_PACKAGE(JPEG REQUIRED)
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})

INCLUDE_DIRECTORIES(${INCLUDE_DIR})
# INCLUDE_DIRECTORIES(${IMAGE_LIB_HEADERS})

add_library(_${PROJECT_NAME} SHARED ${SOURCES} ${IMAGE_LIB_SOURCES})
add_executable(Main ${SOURCES} ${IMAGE_LIB_SOURCES})
target_link_libraries(Main _${PROJECT_NAME} ${PNG_LIBRARY} ${JPEG_LIBRARY})