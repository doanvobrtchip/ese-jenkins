
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h ../reference/*.h resources/*.h)

FILE(GLOB FTRES ../reference/*.h resources/*.h)

SOURCE_GROUP("" FILES ${SRCS} ${HDRS})
SOURCE_GROUP("resources" FILES ${FTRES})

ADD_LIBRARY(ft800emu
  ${SRCS}
  ${HDRS}
)

TARGET_LINK_LIBRARIES(ft800emu
  ft8xxemu_platform)

IF (FTEMU_PIC)
  SET_TARGET_PROPERTIES(ft800emu PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF (FTEMU_PIC)
