
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h ../reference/*.h)

ADD_LIBRARY(ft800emu
  ${SRCS}
  ${HDRS}
)

TARGET_LINK_LIBRARIES(ft800emu
  ft8xxemu_platform)
