include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} /usr/local/jdksmidi2.2-dev/include)
LINK_DIRECTORIES( ${LINK_DIRECTORIES} /usr/local/jdksmidi-2.2-dev/lib )
set(lucifer_SRCS framesequencer.cpp
  staticframe.cpp main.cpp
  framesource_impl.cpp
  loadilda.cpp screendisplay.cpp
  buttongrid.cpp buttonwindow.cpp
  frame.cpp point.cpp driver.cpp
  displayframe.cpp editparams.cpp
  log.cpp colour.cpp
  resampler.cpp
  driver_portaudio_ilda.cpp head.cpp
  driver_dummy_ilda.cpp
  outputview.cpp
  engine.cpp playbacklist.cpp
  qtiocompressor.cpp
  midi.cpp
  controlsurface.cpp
  motormix.cpp
  alsamidi.cpp
  colourrotator.cpp
)

set(lucifer_HDRS framesequencer.h loadilda.h
  framesource.h point.h
  framesource_impl.h
  screendisplay.h frame.h log.h colour.h
  resampler.h
  driver_portaudio_ilda.h

)

set(lucifer_MOC_HDRS screendisplay.h buttongrid.h buttonwindow.h
  displayframe.h editparams.h staticframe.h head.h driver.h
  driver_dummy_ilda.h
  outputview.h
  engine.h playbacklist.h
  engine_impl.h
  qtiocompressor.h
  midi.h
  controlsurface.h
  motormix.h
  alsamidi.h
  colourrotator.h)

SET(lucifer_RCS
  lucifer.qrc
)


QT4_ADD_RESOURCES(lucifer_RC_SRCS ${lucifer_RCS} )
QT4_WRAP_CPP( lucifer_MOC_SRCS ${lucifer_MOC_HDRS} )
add_executable(lucifer ${lucifer_SRCS} ${lucifer_HDRS} ${lucifer_MOC_SRCS} ${lucifer_RC_SRCS})
target_link_libraries(lucifer -lrt -lasound -ljack -lpthread  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} z
	log4cpp portaudio zita-resampler jdksmidi
)
