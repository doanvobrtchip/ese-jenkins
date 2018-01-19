
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(bt8xxemus
  ${SRCS}
  ${HDRS}
)

TARGET_LINK_LIBRARIES(bt8xxemus
  bt8xxemu
)
