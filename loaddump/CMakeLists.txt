
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftloaddump
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftloaddump
	ft800emu
	)

IF (WITH_SDL2)

ELSEIF (WITH_SDL)

  # SDL base package
  find_package (SDL)
  if (NOT SDL_FOUND)
    message (FATAL_ERROR "SDL not found!")
  endif (NOT SDL_FOUND)
  include_directories(
    ${SDL_INCLUDE_DIR}
    )
  target_link_libraries(dump1
    ${SDL_LIBRARY}
    )

  ADD_DEFINITIONS(-DFT800EMU_SDL)

ENDIF ()

add_definitions(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")

