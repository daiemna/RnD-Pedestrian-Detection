CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)

PROJECT(Classifier)

LIST( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -pthread -g -ftest-coverage -fprofile-arcs")

FILE(GLOB_RECURSE SOURCES "src/*.cpp")

INCLUDE_DIRECTORIES("include/")

SET(OPENCV_PATH $ENV{OPENCV_DIR})
SET(OPENCV_LIBRARY  "-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab")

IF(NOT OPENCV_PATH)
	MESSAGE(FATAL_ERROR "OPENCV_DIR envoirnment variable does not exsist")
	exit()
ENDIF()

MESSAGE(${OPENCV_LIBRARY})

SET(OPENCV_INCLUDE_DIR "${OPENCV_PATH}../")

# INCLUDE_DIRECTORIES(${OPENCV_INCLUDE_DIR})
LINK_DIRECTORIES(${OPENCV_LIBRARY_DIRS})


ADD_LIBRARY(_${PROJECT_NAME} SHARED ${SOURCES})
ADD_EXECUTABLE(Main ${SOURCES})
TARGET_LINK_LIBRARIES(Main _${PROJECT_NAME} ${OPENCV_LIBRARY})