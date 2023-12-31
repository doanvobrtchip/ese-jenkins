project(fteditor LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 17)
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

FILE(GLOB BASE
    src/components/*.h
    src/components/*.cpp
)

FILE(GLOB INSPECTOR
    src/components/inspector/*.h
    src/components/inspector/*.cpp
)

FILE(GLOB SCRIPT
    src/components/script/*.h
    src/components/script/*.cpp
)

FILE(GLOB WELCOME
    src/components/welcome/*.h
    src/components/welcome/*.cpp
)


FILE(GLOB REGISTERS
    src/components/registers/*.h
    src/components/registers/*.cpp
)

FILE(GLOB CUSTOMIZE
    src/customize/*.h
    src/customize/*.cpp
)

FILE(GLOB UTILS
    src/utils/*.h
    src/utils/*.cpp
)

FILE(GLOB EXTENSION_MODULE
    src/extension_module/*.cpp
)

FILE(GLOB DEFINE
    src/define/*.h
)

IF (WIN32)
	FILE(GLOB RSRC *.rc)
ENDIF (WIN32)
FILE(GLOB RESOURCES *.qrc)

SOURCE_GROUP("resources" FILES ${RESOURCES} ${RSRC})
SOURCE_GROUP("ftdi" FILES ${SRCSFT} ${HDRSFT})
SOURCE_GROUP("" FILES ${SRCS} ${HDRS})
SOURCE_GROUP("components" FILES ${BASE})
SOURCE_GROUP("components/inspector" FILES ${INSPECTOR})
SOURCE_GROUP("components/script" FILES ${SCRIPT})
SOURCE_GROUP("components/welcome" FILES ${WELCOME})
SOURCE_GROUP("components/registers" FILES ${REGISTERS})
SOURCE_GROUP("customize" FILES ${CUSTOMIZE})
SOURCE_GROUP("utils" FILES ${UTILS})
SOURCE_GROUP("define" FILES ${DEFINE})
SOURCE_GROUP("extension_module" FILES ${EXTENSION_MODULE})

# Find the QtWidgets library, minimum 5.15 or 6.0
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html
find_package(Qt6 QUIET COMPONENTS Widgets LinguistTools Concurrent)
if(Qt6_FOUND AND FTEDITOR_OPENGL_VIEWPORT)
  find_package(Qt6 QUIET COMPONENTS OpenGLWidgets)
endif()
if(NOT Qt6_FOUND)
  find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets LinguistTools Concurrent)
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

INCLUDE_DIRECTORIES(
  .  
  src
  src/components
  ../QScintilla/src
)

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
  ${CUSTOMIZE}
  ${UTILS}
  ${BASE}
  ${INSPECTOR}
  ${REGISTERS}
  ${SCRIPT}
  ${WELCOME}
  ${DEFINE}
  ${EXTENSION_MODULE}
	)

IF(FTEDITOR_DEVICE_MANAGER)
  TARGET_LINK_LIBRARIES(fteditor
    eve_hal
  )
ENDIF()

TARGET_LINK_LIBRARIES(fteditor
	bt8xxemu
	)

TARGET_LINK_LIBRARIES(fteditor
  QScintilla
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
  INCLUDE_DIRECTORIES(
    ${FREETYPE_INCLUDE_DIRS}
    )
  TARGET_LINK_LIBRARIES(fteditor
    ${FREETYPE_LIBRARIES}
    )
  IF (WITH_FREETYPE_STATIC)
    FIND_PACKAGE(ZLIB REQUIRED) # Dependency of Freetype
    FIND_PACKAGE(PNG REQUIRED) # Dependency of Freetype
    TARGET_LINK_LIBRARIES(fteditor
      ${ZLIB_LIBRARIES}
      ${PNG_LIBRARIES}
    )
  ENDIF (WITH_FREETYPE_STATIC)

  ADD_DEFINITIONS(-DFT800EMU_FREETYPE)
ENDIF(WITH_FREETYPE)


target_link_libraries(fteditor Qt::Widgets Qt::Concurrent)
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
    VS_DEBUGGER_ENVIRONMENT "PATH=${BIN_PYTHON_ROOT};${BIN_QT_BINARIES};${BIN_QT_PLUGINS};${BIN_FREETYPE_BINARIES};${BIN_PNG_BINARIES};${BIN_ZLIB_BINARIES};%PATH%\nQT_PLUGIN_PATH=${BIN_QT_PLUGINS}\nFTEDITOR_PYTHONPATH=${BIN_PYTHON_ROOT}")
  # set_target_properties(fteditor PROPERTIES
  #   VS_DEBUGGER_COMMAND_ARGUMENTS "")
  set_target_properties(fteditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/fteditor")
endif()


ADD_DEFINITIONS(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")
ADD_DEFINITIONS(-DQSCINTILLA_DLL)

#Windeployqt
if(WITH_WINDEPLOYQT)
  if(Qt6_FOUND)
    get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
  else()
    get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  endif()
  get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
  set(QT_DEPLOYMENT "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/qt-deployment")
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
  set(QT_DEPLOYMENT_COMMAND_OPTIONS
    $<$<CONFIG:Release>:"--release">$<$<CONFIG:Debug>:"--debug">
    --verbose 0
    --no-system-d3d-compiler
    --no-translations
    --no-compiler-runtime
    --force
    )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${QT_BIN_DIR}" "${WINDEPLOYQT_EXECUTABLE}"
    "$<TARGET_FILE:${PROJECT_NAME}>" ${QT_DEPLOYMENT_COMMAND_OPTIONS}
    --dir ${QT_DEPLOYMENT}
    COMMENT "Running windeployqt ${QT_DEPLOYMENT}..."
    )
endif()
