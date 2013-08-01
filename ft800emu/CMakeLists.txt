
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_LIBRARY(libft800emu
	${SRCS}
	${HDRS}
	)

target_link_libraries(libft800emu
	)

option(WITH_SDL "Build an SDL application." ON)

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
  target_link_libraries(libft800emu
    ${SDL_LIBRARY}
    ${TARGET_LINK_LIBRARIES}
    )

  SET(CMAKE_C_FLAGS "-DFT800EMU_SDL ${PLATFORM_CFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS "-DFT800EMU_SDL ${PLATFORM_CXXFLAGS}" CACHE STRING "" FORCE)

endif (WITH_SDL)
