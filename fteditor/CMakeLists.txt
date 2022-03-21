
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)
IF (WIN32)
	FILE(GLOB RSRC *.rc)
ENDIF (WIN32)
FILE(GLOB RESOURCES *.qrc)

SOURCE_GROUP("resources" FILES ${RESOURCES} ${RSRC})
SOURCE_GROUP("ftdi" FILES ${SRCSFT} ${HDRSFT})
SOURCE_GROUP("" FILES ${SRCS} ${HDRS})

# Find the QtWidgets library
FIND_PACKAGE(Qt5Widgets)

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

QT5_ADD_RESOURCES(RESOURCE_ADDED ${RESOURCES})

IF (FTEDITOR_DEVICE_MANAGER)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/eve_hal
    ${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/include
    ${CMAKE_SOURCE_DIR}/dependencies/ft4222/inc
    ${CMAKE_SOURCE_DIR}/dependencies/ftd2xx
  )
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/x64)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/lib/amd64)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/amd64)
  ELSE()
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/i386)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/lib/i386)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/i386)
  ENDIF()
ENDIF (FTEDITOR_DEVICE_MANAGER)

ADD_EXECUTABLE(fteditor WIN32
	${SRCS}
	${HDRS}
	${SRCSFT}
	${HDRSFT}
	${RSRC}
	${RESOURCE_ADDED}
	)

IF(FTEDITOR_DEVICE_MANAGER)
  TARGET_LINK_LIBRARIES(fteditor
    eve_hal
  )
ENDIF()

TARGET_LINK_LIBRARIES(fteditor
	bt8xxemu
	)

IF (WITH_SDL2)
  ADD_DEFINITIONS(-DFT800EMU_SDL2)
ENDIF ()

IF (WITH_PYTHON)

  FIND_PACKAGE(PythonLibs REQUIRED)
  INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_DIRS}
    )
  TARGET_LINK_LIBRARIES(fteditor
    ${PYTHON_LIBRARIES}
    )

  ADD_DEFINITIONS(-DFT800EMU_PYTHON)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/export_eve_arduino.py
    ${CMAKE_CURRENT_BINARY_DIR}/export_eve_arduino.py COPYONLY)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/export_bt81x.py
    ${CMAKE_CURRENT_BINARY_DIR}/export_bt81x.py COPYONLY)
    
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/export_bt81x_helper.py
    ${CMAKE_CURRENT_BINARY_DIR}/export_bt81x_helper.py COPYONLY)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/img_cvt.py
    ${CMAKE_CURRENT_BINARY_DIR}/img_cvt.py COPYONLY)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/png.py
    ${CMAKE_CURRENT_BINARY_DIR}/png.py COPYONLY)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pngp2pa.py
    ${CMAKE_CURRENT_BINARY_DIR}/pngp2pa.py COPYONLY)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/raw_cvt.py
    ${CMAKE_CURRENT_BINARY_DIR}/raw_cvt.py COPYONLY)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/astc_conv.py
    ${CMAKE_CURRENT_BINARY_DIR}/astc_conv.py COPYONLY)
    
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/helperapi.py
    ${CMAKE_CURRENT_BINARY_DIR}/helperapi.py COPYONLY)
   
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/astcenc.exe
    ${CMAKE_CURRENT_BINARY_DIR}/astcenc.exe COPYONLY)
    
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/EVE_Hal_Library DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/)
  # file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/EVE_Hal_Library DESTINATION  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
  
ENDIF (WITH_PYTHON)

IF(WITH_FREETYPE)
  FIND_PACKAGE(Freetype REQUIRED)
  FIND_PACKAGE(ZLIB REQUIRED) # Dependency of Freetype
  FIND_PACKAGE(PNG REQUIRED) # Dependency of Freetype
  INCLUDE_DIRECTORIES(
    ${FREETYPE_INCLUDE_DIRS}
    )
  TARGET_LINK_LIBRARIES(fteditor
    ${FREETYPE_LIBRARIES}
    )
  IF (WITH_FREETYPE_STATIC)
    TARGET_LINK_LIBRARIES(fteditor
      ${ZLIB_LIBRARIES}
      ${PNG_LIBRARIES}
    )
  ENDIF (WITH_FREETYPE_STATIC)

  ADD_DEFINITIONS(-DFT800EMU_FREETYPE)
ENDIF(WITH_FREETYPE)


IF (WIN32)
  TARGET_LINK_LIBRARIES(fteditor
    Qt5::Widgets
  )
  
  TARGET_LINK_LIBRARIES(fteditor
    version
  )
ELSE (WIN32)
  TARGET_LINK_LIBRARIES(fteditor
    Qt5::Widgets
  )
ENDIF (WIN32)


if(WIN32)
  find_path(BIN_PYTHON_ROOT
    NAMES 
      python.exe python_d.exe
      python3.dll python3_d.dll
      python37.dll python37_d.dll
      python38.dll python38_d.dll
    HINTS
      ${PYTHON_INCLUDE_DIR}/..
  )
  find_path(BIN_QT5_BINARIES
    NAMES 
      Qt5Core.dll Qt5Cored.dll
    HINTS
      ${Qt5Core_DIR}/../../../bin
      ${Qt5Core_DIR}/../../..
  )
  find_path(BIN_QT5_PLUGINS
    NAMES 
      platforms styles imageformats
    HINTS
      ${Qt5Core_DIR}/../../../plugins
      ${Qt5Core_DIR}/../../..
  )
  find_path(BIN_FREETYPE_BINARIES
    NAMES 
      freetype.dll freetyped.dll
    HINTS
      ${FREETYPE_INCLUDE_DIR_freetype2}/../../bin
      ${FREETYPE_INCLUDE_DIR_freetype2}/../..
      ${FREETYPE_INCLUDE_DIR_freetype2}/..
  )
  find_path(BIN_PNG_BINARIES
    NAMES 
      libpng16.dll libpng16d.dll
    HINTS
      ${PNG_PNG_INCLUDE_DIR}/../bin
      ${PNG_PNG_INCLUDE_DIR}/..
  )
  find_path(BIN_ZLIB_BINARIES
    NAMES 
      zlib.dll zlibd.dll
    HINTS
      ${ZLIB_INCLUDE_DIR}/../bin
      ${ZLIB_INCLUDE_DIR}/..
  )
  set_target_properties(fteditor PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${BIN_PYTHON_ROOT};${BIN_QT5_BINARIES};${BIN_QT5_PLUGINS};${BIN_FREETYPE_BINARIES};${BIN_PNG_BINARIES};${BIN_ZLIB_BINARIES};%PATH%\nQT_PLUGIN_PATH=${BIN_QT5_PLUGINS}\nPYTHONPATH=${BIN_PYTHON_ROOT}")
  # set_target_properties(fteditor PROPERTIES
  #   VS_DEBUGGER_COMMAND_ARGUMENTS "")
  set_target_properties(fteditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/fteditor")
endif()


ADD_DEFINITIONS(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")

