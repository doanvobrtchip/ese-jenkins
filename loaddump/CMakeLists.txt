
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftloaddump
  ${SRCS}
  ${HDRS}
)

target_link_libraries(ftloaddump
  ft800emu
)

add_definitions(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")

