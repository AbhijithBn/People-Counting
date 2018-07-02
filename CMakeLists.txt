cmake_minimum_required(VERSION 2.8)
project( DisplayVideo )

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )



add_executable( DisplayVideo main.cpp Blob.cpp Blob.h Morph.h Morph.cpp main.h )
target_link_libraries( DisplayVideo ${OpenCV_LIBS} )

