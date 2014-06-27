########################################################################
# CMake project

# Minimum required 2.6
# Minimum required 2.8.11 for automatic linking to qtmain.lib
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
# set (CMAKE_VERBOSE_MAKEFILE ON)


########################################################################
# Ensure that we are not building in our source directories.

SET(Build_Dir_OK "TRUE")
STRING(REGEX MATCH "^${CMAKE_SOURCE_DIR}" In_Sub_Dir ${CMAKE_BINARY_DIR})
IF (In_Sub_Dir)
  STRING(REGEX MATCH "^${CMAKE_SOURCE_DIR}/build" In_Build_Dir ${CMAKE_BINARY_DIR})
  IF (NOT In_Build_Dir)
    SET(Build_Dir_OK "FALSE")
  ENDIF ()
ENDIF ()

IF (NOT Build_Dir_OK)
  MESSAGE(FATAL_ERROR "You must run cmake from a directory that is not in your source tree, or that is in a special subdirectory of the tree whose name begins with 'build'.")
ENDIF ()


########################################################################
# Set up the basic build environment

IF (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This messes up
  # differentiation between debug and release builds.
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF ()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

########################################################################

OPTION(WITH_SDL2 "Use SDL2 instead of SDL1 (required for FT900EMU)" OFF)

OPTION(FTEDITOR_TEMP_DIR "Store unsaved projects in a temporary directory" OFF)

OPTION(WITH_QT "Build projects that depend on Qt" OFF)

OPTION(WITH_PYTHON "Include editor features that depend on Python" OFF)

OPTION(WITH_FREETYPE "Include features that depend on Freetype" OFF)

OPTION(WITH_FT900EMU "Include FT900EMU" OFF)

########################################################################

IF (NOT DEFINED WITH_SDL)
  IF (WIN32)
    SET(WITH_SDL FALSE CACHE BOOL "Build an SDL application." FORCE)
  ELSE (WIN32)
    SET(WITH_SDL TRUE CACHE BOOL "Build an SDL application." FORCE)
  ENDIF (WIN32)
ENDIF (NOT DEFINED WITH_SDL)

########################################################################

LINK_DIRECTORIES(
  )
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/ft800emu
  ${CMAKE_SOURCE_DIR}/reference
  ${CMAKE_SOURCE_DIR}/arduino
  ${CMAKE_SOURCE_DIR}/editor/ftlib
  )

IF (WITH_SDL2)
  FIND_PACKAGE(SDL2 REQUIRED)
  FIND_PACKAGE(OpenGL REQUIRED)
  LINK_DIRECTORIES(
    )
  INCLUDE_DIRECTORIES(
    ${SDL2_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    )
ENDIF (WITH_SDL2)

########################################################################

ADD_SUBDIRECTORY(ft800emu)
ADD_SUBDIRECTORY(arduino)

ADD_SUBDIRECTORY(loaddump)
ADD_SUBDIRECTORY(app)
ADD_SUBDIRECTORY(dump1)
ADD_SUBDIRECTORY(playxbu)

IF (WITH_QT)
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)
  ADD_SUBDIRECTORY(qtsample)
  ADD_SUBDIRECTORY(editor)
ENDIF (WITH_QT)

IF (WITH_FT900EMU AND WITH_SDL2)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/ft900emu
    )

  ADD_SUBDIRECTORY(ft900emu)

  ADD_SUBDIRECTORY(app900)
ENDIF (WITH_FT900EMU AND WITH_SDL2)

########################################################################
