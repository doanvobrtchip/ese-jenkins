
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_LIBRARY(ft8xxemu_platform
  ${SRCS}
  ${HDRS}
)

TARGET_LINK_LIBRARIES(ft8xxemu_platform
)
