cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(haar_features)

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

SET(FH_IMAGE 
	"../imagelib_lite"
)

SET(FH_IMAGE_LIBRARY 
	../${FH_IMAGE}/build/lib_fh_image_lite.so
)

SET(FH_IMAGE_INCLUDE 
	${FH_IMAGE}/include
)

set (INCLUDE_DIR "")
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIR ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIR)

if(NOT EXISTS ${FH_IMAGE_LIBRARY})
	MESSAGE(FATAL_ERROR "CANNOT FIND fraunhofer image lib at path "${FH_IMAGE_LIBRARY})
endif()

# MESSAGE(STATUS "\nInclude dir: "${FH_IMAGE}/include)

INCLUDE_DIRECTORIES(${INCLUDE_DIR})
INCLUDE_DIRECTORIES(${FH_IMAGE}/include)

add_library(_${PROJECT_NAME} SHARED ${SOURCES})
add_executable(Main ${SOURCES})
target_link_libraries(Main _${PROJECT_NAME} ${FH_IMAGE_LIBRARY})