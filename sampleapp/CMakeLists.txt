
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftsampleapp
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftsampleapp
	libft800emu
	libftarduino
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

