########################################################################
# CMake project
PROJECT("FTEMU")

# Minimum required 2.6
# Minimum required 2.8.11 for automatic linking to qtmain.lib
# Minimum required 3.1 for c++ standard macro
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)
# SET(CMAKE_VERBOSE_MAKEFILE ON)

IF (POLICY CMP0020)
	CMAKE_POLICY(SET CMP0020 NEW)
ENDIF ()

SET(CMAKE_CXX_STANDARD 14)


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
# Strip binary in release mode

IF (CMAKE_COMPILER_IS_GNUCXX AND !MINGW)
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
ENDIF ()

########################################################################

IF (WIN32)
  OPTION(WITH_SDL2 "Use SDL2 (required for FT900EMU)" OFF)
ELSE ()
  OPTION(WITH_SDL2 "Use SDL2 (required for FT900EMU and Linux support)" ON)
ENDIF ()

OPTION(FTEDITOR_TEMP_DIR "Store unsaved projects in a temporary directory" ON)

OPTION(FTEDITOR_DEVICE_MANAGER "Enable the device manager" ON)

OPTION(FTEDITOR_LINK_STATIC "Link statically with emulator" ON)

OPTION(WITH_QT "Build projects that depend on Qt" OFF)

OPTION(WITH_PYTHON "Include editor features that depend on Python" OFF)

OPTION(WITH_FREETYPE "Include features that depend on Freetype" OFF)

OPTION(WITH_FREETYPE_STATIC "Include dependencies for static Freetype build" OFF)

OPTION(WITH_BT8XXEMU "Include BT8XXEMU" ON)

OPTION(WITH_FT800EMU "Include FT800EMU" ON)

OPTION(WITH_FT810EMU "Include FT810EMU" ON)

OPTION(WITH_BT815EMU "Include BT815EMU" ON)

OPTION(WITH_FT900EMU "Include FT900EMU" ON)

OPTION(WITH_MX25LEMU "Include MX25LEMU" ON)

OPTION(WITH_SAMPLEAPP "Include SampleApp" OFF)

OPTION(WITH_EMUTEST "Include emulator test projects" ON)

OPTION(WITH_EMUSERVICE "Include emulator service" ON)

SET(SDL_ASSERT_LEVEL 1 CACHE INTEGER "Assert level for SDL2")

########################################################################

IF(FTEDITOR_TEMP_DIR)
  ADD_DEFINITIONS(-DFTEDITOR_TEMP_DIR)
ENDIF(FTEDITOR_TEMP_DIR)

IF(FTEDITOR_DEVICE_MANAGER)
  ADD_DEFINITIONS(-DFTEDITOR_DEVICE_MANAGER)
ENDIF(FTEDITOR_DEVICE_MANAGER)

ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

########################################################################

LINK_DIRECTORIES(
  )
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/astc
  ${CMAKE_SOURCE_DIR}/ft8xxemu_platform
  ${CMAKE_SOURCE_DIR}/ft800emu
  ${CMAKE_SOURCE_DIR}/ft810emu
  ${CMAKE_SOURCE_DIR}/bt815emu
  ${CMAKE_SOURCE_DIR}/bt8xxemu
  ${CMAKE_SOURCE_DIR}/reference
  ${CMAKE_SOURCE_DIR}/fteditor/ftlib
  )

IF (WITH_SDL2)
  FIND_PACKAGE(SDL2 REQUIRED)
  FIND_PACKAGE(OpenGL REQUIRED)
  LINK_DIRECTORIES(
    )
  INCLUDE_DIRECTORIES(
    ${SDL2_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/gl3w
    )
  ADD_DEFINITIONS(-DSDL_ASSERT_LEVEL${SDL_ASSERT_LEVEL})
  ADD_DEFINITIONS(-DFTEMU_SDL2)
ENDIF (WITH_SDL2)

########################################################################

option (USE_SSE_41 "Use SSE 4.1 instructions" OFF)

IF (USE_SSE_41)
  IF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    SET (CMAKE_C_FLAGS "-msse4.1 ${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
    SET (CMAKE_CXX_FLAGS "-msse4.1 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
  ENDIF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  
  ADD_DEFINITIONS(-DFTEMU_SSE41=1)
ELSE (USE_SSE_41)
  ADD_DEFINITIONS(-DFTEMU_SSE41=0)
ENDIF (USE_SSE_41)

########################################################################

ADD_DEFINITIONS(-DFTEMU_CMAKE)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

IF (WITH_FT900EMU)
  ADD_DEFINITIONS(-DFTEMU_HAVE_FT900EMU)
ENDIF (WITH_FT900EMU)

########################################################################

IF (WITH_SDL2)
  ADD_SUBDIRECTORY(gl3w)
ENDIF (WITH_SDL2)

IF (WITH_BT8XXEMU)
  ADD_SUBDIRECTORY(ft8xxemu_platform)
  IF (WITH_FT800EMU)
    ADD_SUBDIRECTORY(ft800emu)
    ADD_DEFINITIONS(-DFTEMU_HAVE_FT800EMU)
  ENDIF (WITH_FT800EMU)
  IF (WITH_FT810EMU)
    ADD_SUBDIRECTORY(ft810emu)
    ADD_DEFINITIONS(-DFTEMU_HAVE_FT810EMU)
  ENDIF (WITH_FT810EMU)
  IF (WITH_BT815EMU)
    ADD_SUBDIRECTORY(astc)
    ADD_SUBDIRECTORY(bt815emu)
    ADD_DEFINITIONS(-DFTEMU_HAVE_BT815EMU)
  ENDIF (WITH_BT815EMU)
  ADD_SUBDIRECTORY(bt8xxemu)
  ADD_SUBDIRECTORY(bt8xxemu_static)

  IF (WITH_EMUTEST)
    ADD_SUBDIRECTORY(ftemutest)
  ENDIF (WITH_EMUTEST)

  ADD_SUBDIRECTORY(testflash)
  ADD_SUBDIRECTORY(dump1)
  ADD_SUBDIRECTORY(dump2)
  ADD_SUBDIRECTORY(dump3)

  IF (WITH_EMUSERVICE)
    ADD_SUBDIRECTORY(bt8xxemus)
    ADD_SUBDIRECTORY(playxbuc)
  ENDIF (WITH_EMUSERVICE)

  #ADD_SUBDIRECTORY(loaddump)
  ADD_SUBDIRECTORY(playxbu)

  IF (WITH_QT)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)
    ADD_SUBDIRECTORY(fteditor)
  ENDIF (WITH_QT)
ENDIF (WITH_BT8XXEMU)

IF (WITH_MX25LEMU)
  ADD_SUBDIRECTORY(mx25lemu)
ENDIF (WITH_MX25LEMU)

IF (WITH_FT900EMU AND WITH_SDL2)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/ft900emu
    )

  ADD_SUBDIRECTORY(ft900emu)

  IF (WITH_BT8XXEMU)
    ADD_SUBDIRECTORY(ft900eve)
  ENDIF (WITH_BT8XXEMU)
ENDIF (WITH_FT900EMU AND WITH_SDL2)

########################################################################
