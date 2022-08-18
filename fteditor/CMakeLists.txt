
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)
IF (WIN32)
	FILE(GLOB RSRC *.rc)
ENDIF (WIN32)
FILE(GLOB RESOURCES *.qrc)

SOURCE_GROUP("resources" FILES ${RESOURCES} ${RSRC})
SOURCE_GROUP("ftdi" FILES ${SRCSFT} ${HDRSFT})
SOURCE_GROUP("" FILES ${SRCS} ${HDRS})

# Find the QtWidgets library, minimum 5.15 or 6.0
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html
find_package(Qt6 QUIET COMPONENTS Widgets LinguistTools)
if(Qt6_FOUND AND FTEDITOR_OPENGL_VIEWPORT)
  find_package(Qt6 QUIET COMPONENTS OpenGLWidgets)
endif()
if(NOT Qt6_FOUND)
  find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets LinguistTools)
  if(FTEDITOR_OPENGL_VIEWPORT)
    find_package(Qt5 5.15 REQUIRED COMPONENTS OpenGLWidgets)
  endif()
endif()


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

if(FTEDITOR_GENERATE_TRANSLATIONS)
  qt_create_translation(QM_FILES ${SRCS} ${HDRS} translation_en.ts)
else()
  qt_add_translation(QM_FILES translation_en.ts)
endif()
configure_file(translations.qrc.in translations.qrc COPYONLY)

qt_add_resources(RESOURCE_ADDED ${RESOURCES} ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)


IF (FTEDITOR_DEVICE_MANAGER)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/eve_hal
    ${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/include
    ${CMAKE_SOURCE_DIR}/dependencies/ft4222/inc
    ${CMAKE_SOURCE_DIR}/dependencies/ftd2xx
  )
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/x64)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/dll/amd64)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/amd64)
  ELSE()
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/i386)
    LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/dll/i386)
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


target_link_libraries(fteditor Qt::Widgets)
if(WIN32)
  target_link_libraries(fteditor version)
endif()
if(FTEDITOR_OPENGL_VIEWPORT)
  target_link_libraries(fteditor Qt6::OpenGLWidgets)
  ADD_DEFINITIONS(-DFTEDITOR_OPENGL_VIEWPORT)
endif()

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

  find_path(BIN_QT_BINARIES
    NAMES 
      Qt6Core.dll Qt6Cored.dll
      Qt5Core.dll Qt5Cored.dll
    HINTS
      ${Qt6Core_DIR}/../../../bin
      ${Qt6Core_DIR}/../../..
      ${Qt5Core_DIR}/../../../bin
      ${Qt5Core_DIR}/../../..
  )
  find_path(BIN_QT_PLUGINS

    NAMES 
      platforms styles imageformats
    HINTS
      ${Qt6Core_DIR}/../../../plugins
      ${Qt6Core_DIR}/../../..

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

    VS_DEBUGGER_ENVIRONMENT "PATH=${BIN_PYTHON_ROOT};${BIN_Qt6_BINARIES};${BIN_Qt6_PLUGINS};${BIN_FREETYPE_BINARIES};${BIN_PNG_BINARIES};${BIN_ZLIB_BINARIES};%PATH%\nQT_PLUGIN_PATH=${BIN_Qt6_PLUGINS}\nPYTHONPATH=${BIN_PYTHON_ROOT}")
  # set_target_properties(fteditor PROPERTIES
  #   VS_DEBUGGER_COMMAND_ARGUMENTS "")
  set_target_properties(fteditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/fteditor")
endif()


ADD_DEFINITIONS(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")

