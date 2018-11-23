cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME "record" )

project (${PROJECT_NAME})

# Please insert your OpenCV path
set(CMAKE_MODULE_PATH "/opt/opencv331/opencv")
set(ROYALE_DIR "/home/user/libroyale-3.11.0.42-LINUX-x86-64Bit")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include" )
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src" )

#OpenCv
find_package(OpenCV QUIET)
if(NOT OpenCV_FOUND)
  message("OpenCV example will not be build as no OpenCV was found!")
  return()
endif()

#Royale
set(CMAKE_PREFIX_PATH "${ROYALE_DIR}/share")
find_package(royale REQUIRED)
link_directories(${royale_LIB_DIR})

#Boost
set( Boost_USE_STATIC_LIBS ON )
set( Boost_USE_STATIC ON )
find_package(Boost 1.58.0 COMPONENTS filesystem REQUIRED)

# Includes
include_directories(
  ${INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

# Project Files
set(
  HEADERS
  ${INCLUDE_DIR}/CamListener.h
)

set(
  SOURCES 
  ${SRC_DIR}/CamListener.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} record_main.cpp)
target_link_libraries(${PROJECT_NAME} "${royale_LIBS}" "${OpenCV_LIBRARIES}" "${Boost_LIBRARIES}") 
COPY_ROYALE_LIBS(${PROJECT_NAME}) 

add_executable("replay" ${SOURCES} replay_main.cpp)
target_link_libraries("replay" "${royale_LIBS}" "${OpenCV_LIBRARIES}" "${Boost_LIBRARIES}") 
COPY_ROYALE_LIBS("replay")  



