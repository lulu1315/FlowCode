CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

add_custom_target(
   imagelib
   COMMAND make
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/imageLib
)

PROJECT(FloCode)

#find_package(PNG REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

include_directories(${PNG_INCLUDE_DIR})

ADD_EXECUTABLE(exr2flo
"${CMAKE_SOURCE_DIR}/exr2flo.cpp" 
)

ADD_EXECUTABLE(flo2exr
"${CMAKE_SOURCE_DIR}/flo2exr.cpp" 
)

ADD_EXECUTABLE(flo2png
"${CMAKE_SOURCE_DIR}/flo2png.cpp" 
)

ADD_EXECUTABLE(deepflow_opencv
"${CMAKE_SOURCE_DIR}/deepflow_opencv.cpp" 
)

ADD_EXECUTABLE(deepflow_consistency
"${CMAKE_SOURCE_DIR}/deepflow_consistency.cpp" 
)

TARGET_LINK_LIBRARIES(exr2flo ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(flo2exr ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(flo2png ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(deepflow_opencv ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(deepflow_consistency ${OpenCV_LIBS})

#add_dependencies(exr2flo imagelib)
#add_dependencies(flo2exr imagelib)
#add_dependencies(flo2png imagelib)

#https://cmake.org/pipermail/cmake/2010-November/040631.html
