cmake_minimum_required(VERSION 2.8) 
PROJECT (FaceGame)
find_package(OpenCV REQUIRED )

file( GLOB NAME_SRC "src/*.cpp" )

file( GLOB NAME_HEADERS "include/*.h" )

#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include )
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include
                                  ${OpenCV_INCLUDE_DIRS})

link_directories( ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable( FaceGame ${NAME_SRC} ${NAME_HEADERS} )

target_link_libraries( FaceGame ${OpenCV_LIBS} )