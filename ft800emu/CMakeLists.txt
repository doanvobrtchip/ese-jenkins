
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_LIBRARY(libft800emu
	${SRCS}
	${HDRS}
	)

target_link_libraries(libft800emu
	)

option(WITH_SDL "Build an SDL application." !WIN32)

if (WITH_SDL)

  # SDL base package
  find_package (SDL)
  if (NOT SDL_FOUND)
    message(FATAL_ERROR "SDL not found!")
  endif (NOT SDL_FOUND)
  include_directories(
    ${SDL_INCLUDE_DIR}
    ${INCLUDE_DIRECTORIES}
    )
  target_link_libraries(libft800emu
    ${SDL_LIBRARY}
    ${TARGET_LINK_LIBRARIES}
    )
  add_definitions(-DFT800EMU_SDL)

else (WITH_SDL)

  FIND_PACKAGE(OpenMP REQUIRED)
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  endif()

endif (WITH_SDL)

option (USE_SSE_41 "Use SSE 4.1 instructions" OFF)

if (USE_SSE_41)

  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_C_FLAGS "-msse4.1 ${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
    SET(CMAKE_CXX_FLAGS "-msse4.1 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
  endif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  
  add_definitions(-DFT800EMU_SSE41=1)

else (USE_SSE_41)

  add_definitions(-DFT800EMU_SSE41=0)

endif (USE_SSE_41)

add_definitions(-DFT800EMU_CMAKE)

