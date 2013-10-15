########################################################################
# CMake project

# Minimum required 2.6
# Minimum required 2.8.11 for automatic linking to qtmain.lib
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
# set (CMAKE_VERBOSE_MAKEFILE ON)


########################################################################
# Ensure that we are not building in our source directories.

set (Build_Dir_OK "TRUE")
string (REGEX MATCH "^${CMAKE_SOURCE_DIR}" In_Sub_Dir ${CMAKE_BINARY_DIR})
if (In_Sub_Dir)
  string (REGEX MATCH "^${CMAKE_SOURCE_DIR}/build" In_Build_Dir ${CMAKE_BINARY_DIR})
  if (NOT In_Build_Dir)
    set (Build_Dir_OK "FALSE")
  endif ()
endif ()

if (NOT Build_Dir_OK)
  message (FATAL_ERROR "You must run cmake from a directory that is not in your source tree, or that is in a special subdirectory of the tree whose name begins with ‘build’.")
endif ()


########################################################################
# Set up the basic build environment

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This messes up
  # differentiation between debug and release builds.
  set (CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

########################################################################

link_directories(
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/ft800emu
  ${CMAKE_SOURCE_DIR}/reference
  ${CMAKE_SOURCE_DIR}/arduino
  )

ADD_SUBDIRECTORY(ft800emu)
ADD_SUBDIRECTORY(arduino)

ADD_SUBDIRECTORY(loaddump)
ADD_SUBDIRECTORY(app)
ADD_SUBDIRECTORY(dump1)
ADD_SUBDIRECTORY(playxbu)

OPTION(WITH_QT "Build projects that depend on Qt" OFF)

IF (WITH_QT)
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)
  ADD_SUBDIRECTORY(qtsample)
ENDIF (WITH_QT)

########################################################################
