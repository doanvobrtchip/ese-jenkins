
FILE(GLOB SRCS *.cpp *.c)
FILE(GLOB HDRS *.h)
FILE(GLOB RSCS *.rc)

ADD_LIBRARY(ft8xxemu SHARED
  ${SRCS}
  ${HDRS}
  ${RSCS}
)

TARGET_LINK_LIBRARIES(ft8xxemu
  bt8xxemu
)

IF (MINGW)
  SET_TARGET_PROPERTIES(ft8xxemu PROPERTIES
    PREFIX "")
ENDIF (MINGW)

SET_TARGET_PROPERTIES(ft8xxemu PROPERTIES
  COMPILE_DEFINITIONS "FT8XXEMU_EXPORT_DYNAMIC")
