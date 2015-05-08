cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(cascade_framework)

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

set (INCLUDE_DIR "")
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIR ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIR)

# FIND_PACKAGE(PNG REQUIRED)
# INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

# FIND_PACKAGE(JPEG REQUIRED)
# INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})

FIND_PATH(FH_IMAGE_LIB_INCLUDE 
	"../imagelib_lite/include"
)

INCLUDE_DIRECTORIES(${INCLUDE_DIR})



add_library(_${PROJECT_NAME} SHARED ${SOURCES})
add_executable(Main ${SOURCES})
target_link_libraries(Main _${PROJECT_NAME} )