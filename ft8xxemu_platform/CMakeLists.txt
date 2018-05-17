
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

SOURCE_GROUP("" FILES ${SRCS} ${HDRS})

ADD_LIBRARY(ft8xxemu_platform
  ${SRCS}
  ${HDRS}
)

IF (MINGW)
  TARGET_LINK_LIBRARIES(ft8xxemu_platform
    avrt dinput8 dxguid)
ELSE (MINGW)
  TARGET_LINK_LIBRARIES(ft8xxemu_platform
    )
ENDIF (MINGW)

IF (WITH_SDL2)
  TARGET_LINK_LIBRARIES(ft8xxemu_platform
    ${SDL2_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    gl3w
    )
ELSEIF (WITH_SDL)
  TARGET_LINK_LIBRARIES(ft8xxemu_platform
    ${SDL_LIBRARY}
    )
ENDIF ()

TARGET_LINK_LIBRARIES(ft8xxemu_platform
)

IF (FTEMU_PIC)
  SET_TARGET_PROPERTIES(ft8xxemu_platform PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF (FTEMU_PIC)
