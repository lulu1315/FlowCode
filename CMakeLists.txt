CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

add_custom_target(
   imagelib
   COMMAND make
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/imageLib
)

PROJECT(FloCode)

find_package(PNG REQUIRED)

include_directories(${PNG_INCLUDE_DIR})

ADD_EXECUTABLE(exr2flo
"${CMAKE_SOURCE_DIR}/exr2flo.cpp" 
"${CMAKE_SOURCE_DIR}/tinyexr/tinyexr.cc" 
"${CMAKE_SOURCE_DIR}/flowIO.cpp" 
)

ADD_EXECUTABLE(flo2exr
"${CMAKE_SOURCE_DIR}/flo2exr.cpp" 
"${CMAKE_SOURCE_DIR}/tinyexr/tinyexr.cc" 
"${CMAKE_SOURCE_DIR}/flowIO.cpp" 
)

ADD_EXECUTABLE(flo2png
"${CMAKE_SOURCE_DIR}/color_flow.cpp" 
"${CMAKE_SOURCE_DIR}/flowIO.cpp" 
"${CMAKE_SOURCE_DIR}/colorcode.cpp" 
)

TARGET_LINK_LIBRARIES(exr2flo ${CMAKE_SOURCE_DIR}/imageLib/libImg.x86_64-g.a)
TARGET_LINK_LIBRARIES(flo2exr ${CMAKE_SOURCE_DIR}/imageLib/libImg.x86_64-g.a)
TARGET_LINK_LIBRARIES(flo2png ${CMAKE_SOURCE_DIR}/imageLib/libImg.x86_64-g.a ${PNG_LIBRARY})

add_dependencies(exr2flo imagelib)
add_dependencies(flo2exr imagelib)
add_dependencies(flo2png imagelib)

#https://cmake.org/pipermail/cmake/2010-November/040631.html
