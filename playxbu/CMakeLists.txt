
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(btplayxbu
  ${SRCS}
  ${HDRS}
)

target_link_libraries(btplayxbu
  bt8xxemu
)

add_definitions(-DFT800EMU_REFERENCE_DIR="${FT800_REFERENCE_DIR}")
