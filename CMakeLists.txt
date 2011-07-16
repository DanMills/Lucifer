PROJECT(lucifer)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
ADD_DEFINITIONS( -Wall -Wextra)
ADD_DEFINITIONS( -W -O3 -g -ffast-math)
INCLUDE( ${QT_USE_FILE} )

add_subdirectory ( src )

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} /usr/local/jdksmidi2.2-dev/include)
LINK_DIRECTORIES( ${LINK_DIRECTORIES} /usr/local/jdksmidi-2.2-dev/lib )
