PROJECT(lucifer)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
ADD_DEFINITIONS( -Wall )
ADD_DEFINITIONS( -W -O3 -g -ffast-math)
INCLUDE( ${QT_USE_FILE} )

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(lucifer_SRCS src/framesequencer.cpp
	src/framesource.cpp src/staticframe.cpp src/main.cpp
	src/loadilda.cpp src/screendisplay.cpp
	src/buttongrid.cpp src/buttonwindow.cpp
	src/point.cpp src/frame.cpp
	src/displayframe.cpp src/editparams.cpp
	src/log.cpp src/frameroot.cpp)

set(lucifer_HDRS src/framesequencer.h src/loadilda.h
	src/framesource.h src/point.h
	src/screendisplay.h src/frame.h src/log.h src/frameroot.h)

set(lucifer_MOC_HDRS src/screendisplay.h src/buttongrid.h src/buttonwindow.h
src/displayframe.h src/editparams.h src/staticframe.h)

SET(lucifer_RCS
	src/lucifer.qrc
)


QT4_ADD_RESOURCES(lucifer_RC_SRCS ${lucifer_RCS} )
QT4_WRAP_CPP( lucifer_MOC_SRCS ${lucifer_MOC_HDRS} )
add_executable(lucifer ${lucifer_SRCS} ${lucifer_HDRS} ${lucifer_MOC_SRCS} ${lucifer_RC_SRCS})
target_link_libraries(lucifer ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} z log4cpp)
