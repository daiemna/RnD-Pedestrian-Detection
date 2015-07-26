CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
PROJECT(Detector)

LIST( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -pthread -g -ftest-coverage -fprofile-arcs")

SET(FEATURES "/home/dna/eclipse_workspace/RnD-Pedestrian-Detection/Features/")
SET(CLASSIFIER "/home/dna/eclipse_workspace/RnD-Pedestrian-Detection/Classifier/")

# INCLUDE(${FEATURES}CMakeLists.txt)
# INCLUDE(${CLASSIFIER}CMakeLists.txt)

# FILE(GLOB_RECURSE FEATURE_SOURCES ${FEATURES}"src/*.cpp")
INCLUDE_DIRECTORIES(${FEATURES}include/)
SET(FEATURE_SOURCES ${FEATURES}src/)

# FILE(GLOB_RECURSE CLASSIFIER_SOURCES ${CLASSIFIER}"src/*.cpp")
INCLUDE_DIRECTORIES(${CLASSIFIER}include/)
SET(CLASSIFIER_SOURCES ${CLASSIFIER}src/)

FILE(GLOB_RECURSE SOURCES "src/*.cpp")
INCLUDE_DIRECTORIES("include/")

SET(SOURCES 
	${SOURCES} 
	${CLASSIFIER_SOURCES}AbstractClassifier.cpp 
	${CLASSIFIER_SOURCES}CascadedClassifier.cpp
	${FEATURE_SOURCES}FeatureEvaluator.cpp
	${FEATURE_SOURCES}hog_features.cpp
	${FEATURE_SOURCES}hog_lite_features.cpp)

# SET(OPENCV_PATH $ENV{OPENCV_DIR})
# IF(NOT OPENCV_PATH)
# 	MESSAGE(FATAL_ERROR "OPENCV_DIR envoirnment variable does not exsist")
# 	exit()
# ENDIF()
# MESSAGE("opencv dir : ${OPENCV_PATH}")

# SET(OPENCV_INCLUDE_DIR "${OPENCV_PATH}")
# SET(OPENCV_LIBRARY  "-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab")
# SET(OPENCV_LIBRARY  "cv2 libopencv_calib3d libopencv_core libopencv_features2d libopencv_flann libopencv_highgui libopencv_imgcodecs libopencv_imgproc libopencv_java300 libopencv_ml libopencv_objdetect libopencv_photo libopencv_shape libopencv_stitching libopencv_superres libopencv_video libopencv_videoio libopencv_videostab libopencv_viz")
# "cv2" "libopencv_java300"
# SET(OPENCV_LIBRARY_LIST "libopencv_calib3d" "libopencv_core" "libopencv_features2d" "libopencv_flann" "libopencv_highgui" "libopencv_imgcodecs" "libopencv_imgproc" "libopencv_ml" "libopencv_objdetect" "libopencv_photo" "libopencv_shape" "libopencv_stitching" "libopencv_superres" "libopencv_video" "libopencv_videoio" "libopencv_videostab" "libopencv_viz")
# SET(OPENCV_APP_INCLUDE "${OPENCV_PATH}/apps")

# SET(CLASSIFIER_LIBRARY "${CLASSIFIER}build/lib_Classifier.so")
# SET(FEATURES_LIBRARY "${FEATURES}build/lib_Features.so")
# SET(OPENCV_LIBRARY_DIR "${OPENCV_PATH}/build/lib")

# INCLUDE_DIRECTORIES(${OPENCV_INCLUDE_DIR})
# INCLUDE_DIRECTORIES(${OPENCV_APP_INCLUDE})

# LINK_DIRECTORIES(${OPENCV_LIBRARY_DIR})
# LINK_DIRECTORIES("${CLASSIFIER}build/")
# LINK_DIRECTORIES("${FEATURES}build/")

# set_target_properties

FIND_PACKAGE( OpenCV REQUIRED )

ADD_LIBRARY(_${PROJECT_NAME} SHARED ${SOURCES})
ADD_EXECUTABLE(Main ${SOURCES})
TARGET_LINK_LIBRARIES(Main _${PROJECT_NAME})
TARGET_LINK_LIBRARIES(Main ${OpenCV_LIBS} )
# MESSAGE("INCLUDING OPENCV LIBS!!!")
# FOREACH(OCV_LIB ${OPENCV_LIBRARY_LIST})
	# MESSAGE("LIB : ${OCV_LIB}.a")
	# ADD_LIBRARY(${OCV_LIB} SHARED IMPORTED)
# 	SET_TARGET_PROPERTIES( ${OCV_LIB} PROPERTIES IMPORTED_LOCATION ${OPENCV_LIBRARY_DIR}/${OCV_LIB}.so)
	# TARGET_LINK_LIBRARIES(Main PRIVATE "${OCV_LIB}_pch_dephelp.a")
# ENDFOREACH()
# TARGET_LINK_LIBRARIES(Main ${OPENCV_LIBRARY})
# TARGET_LINK_LIBRARIES(Main "${OPENCV_LIBRARY_DIR}/cv2.so")