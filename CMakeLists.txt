cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(pedestrian_detection)

set(OPENCV_PATH $ENV{OPENCV_DIR})
if(NOT OPENCV_PATH)
	message(FATAL_ERROR "OPENCV_DIR envoirnment variable does not exsist")
	exit()
endif()

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set(OPENCV_INCLUDE_DIRS ${OPENCV_PATH})
set(OPENCV_LIBRARY_DIRS "${OPENCV_PATH}../../lib/")
set(OPENCV_LIBRARIES  "-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_ts -lopencv_video -lopencv_videostab")

include_directories(${OPENCV_INCLUDE_DIRS})
link_directories(${OPENCV_LIBRARY_DIRS})

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

set (INCLUDE_DIR "")
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIR ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIR)

include_directories(${INCLUDE_DIR})
add_executable (Main ${SOURCES})
target_link_libraries (Main ${OPENCV_LIBRARIES})
