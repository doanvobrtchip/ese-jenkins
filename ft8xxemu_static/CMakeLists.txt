
FILE(GLOB SRCS ../ft8xxemu/*.cpp)
FILE(GLOB HDRS ../ft8xxemu/*.h)

ADD_LIBRARY(ft8xxemu_static
  ${SRCS}
  ${HDRS}
)

TARGET_LINK_LIBRARIES(ft8xxemu_static
  ft800emu
  ft810emu
)

SET_TARGET_PROPERTIES(ft8xxemu_static PROPERTIES
  COMPILE_DEFINITIONS "FT8XXEMU_STATIC")
