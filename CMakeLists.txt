cmake_minimum_required(VERSION 2.8)
project( egbis-opencv )

# Add the EGBIS algortihm
# add_subdirectory(egbis)

find_package( OpenCV 2.4 REQUIRED )
add_executable( main 
  main.cpp  
  egbis/segment-image.hpp 
  egbis/segment-graph.hpp 
  egbis/disjoint-set.h 
  egbis/image.h)
target_link_libraries( main ${OpenCV_LIBS} )

