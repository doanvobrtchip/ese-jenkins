
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(dump1
	${SRCS}
	${HDRS}
	)

target_link_libraries(dump1
	libft800emu
	)

if (WITH_SDL)

  # SDL base package
  find_package (SDL)
  if (NOT SDL_FOUND)
    message (FATAL_ERROR "SDL not found!")
  endif (NOT SDL_FOUND)
  include_directories(
    ${SDL_INCLUDE_DIR}
    ${INCLUDE_DIRECTORIES}
    )
  target_link_libraries(dump1
    ${SDL_LIBRARY}
    ${TARGET_LINK_LIBRARIES}
    )

  SET(CMAKE_C_FLAGS "-DFT800EMU_SDL ${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS "-DFT800EMU_SDL ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)

endif (WITH_SDL)

add_definitions(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")

