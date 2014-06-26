
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

ADD_EXECUTABLE(ftqtsample
	${SRCS}
	${HDRS}
	)

TARGET_LINK_LIBRARIES(ftqtsample
	libft800emu
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

# Find the QtWidgets library
FIND_PACKAGE(Qt5Widgets)

TARGET_LINK_LIBRARIES(ftqtsample
  Qt5::Widgets
)

qt5_use_modules(ftqtsample Widgets)

ADD_DEFINITIONS(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")

