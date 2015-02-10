
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftapp
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftapp
	ft8xxemuc
	ftarduino
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
  target_link_libraries(ftapp
    ${SDL_LIBRARY}
    )
  ADD_DEFINITIONS(-DFT800EMU_SDL)
ENDIF ()

