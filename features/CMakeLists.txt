cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(features)

# SET(CMAKE_CXX_FLAGS 
# 	"-pthread"
# )

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -pthread -g -ftest-coverage -fprofile-arcs")

file(GLOB_RECURSE SOURCES "src/*.cpp")
# file(GLOB_RECURSE HEADERS "include/*.h")

SET(FH_IMAGE 
	"/home/dna/eclipse_workspace/RnD-Pedestrian-Detection/imagelib_lite/"
)

SET(FH_IMAGE_LIBRARY 
	${FH_IMAGE}/build/lib_fh_image_lite.so
)

SET(FH_IMAGE_INCLUDE 
	${FH_IMAGE}/include/image_lib
)
SET (INCLUDE_DIR 
	"include/*.h"
)

if(NOT EXISTS ${FH_IMAGE_LIBRARY})
	MESSAGE(FATAL_ERROR "CANNOT FIND fraunhofer image lib at path " ${FH_IMAGE_LIBRARY})
else()
	MESSAGE(STATUS "FOUND fraunhofer image lib at path " ${FH_IMAGE_LIBRARY})
endif()

if(NOT EXISTS ${FH_IMAGE_INCLUDE}/image.h)
	MESSAGE(FATAL_ERROR "CANNOT FIND fraunhofer image headers at path " ${FH_IMAGE_INCLUDE})
else()
	MESSAGE(STATUS "FOUND fraunhofer image headers at path " ${FH_IMAGE_INCLUDE})
endif()

# MESSAGE(STATUS "\nInclude dir: "${FH_IMAGE}"/include")

INCLUDE_DIRECTORIES("include/")
INCLUDE_DIRECTORIES(${INCLUDE_DIR})
INCLUDE_DIRECTORIES(${FH_IMAGE_INCLUDE})


FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

FIND_PACKAGE(JPEG REQUIRED)
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})

set(OPENCV_PATH $ENV{OPENCV_DIR})
if(NOT OPENCV_PATH)
	message(FATAL_ERROR "OPENCV_DIR envoirnment variable does not exsist")
	exit()
endif()

set(OPENCV_INCLUDE_DIR "${OPENCV_PATH}../")
set(OPENCV_LIBRARY_DIR "${OPENCV_PATH}../../lib/")
set(OPENCV_LIBRARY  "-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab")

include_directories(${OPENCV_INCLUDE_DIR})
link_directories(${OPENCV_LIBRARY_DIRS})


add_library(_${PROJECT_NAME} SHARED ${SOURCES})
add_executable(Main ${SOURCES} ${FH_IMAGE_SOURCES})
target_link_libraries(Main _${PROJECT_NAME} ${FH_IMAGE_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARY} ${OPENCV_LIBRARY})