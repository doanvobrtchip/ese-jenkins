
FILE(GLOB SRCS *.c)
FILE(GLOB HDRS *.h)
SOURCE_GROUP("" FILES ${SRCS} ${HDRS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ftd2xx)

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/x64)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/dll/amd64)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/amd64)
ELSE()
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/i386)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ft4222/dll/i386)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/i386)
ENDIF()

ADD_DEFINITIONS(-DEVE_HAL_EXPORT)

ADD_LIBRARY(eve_hal SHARED
  ${SRCS}
  ${HDRS}
  ${COMP}
)

TARGET_LINK_LIBRARIES(eve_hal
  libMPSSE
  ftd2xx
  bt8xxemu
)

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
TARGET_LINK_LIBRARIES(eve_hal
  LibFT4222-64
)
ELSE()
TARGET_LINK_LIBRARIES(eve_hal
  LibFT4222
)
ENDIF()

IF (WIN32)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ADD_CUSTOM_COMMAND(TARGET eve_hal POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/x64/libMPSSE.dll
      ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/libMPSSE.dll)
    ADD_CUSTOM_COMMAND(TARGET eve_hal POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/dependencies/ft4222/dll/amd64/LibFT4222-64.dll
      ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/LibFT4222-64.dll)
    ADD_CUSTOM_COMMAND(TARGET eve_hal POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/amd64/ftd2xx.dll
      ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/ftd2xx.dll)
  ELSE()
    ADD_CUSTOM_COMMAND(TARGET eve_hal POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/dependencies/mpsse_spi/lib/windows/i386/libMPSSE.dll
      ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/libMPSSE.dll)
    ADD_CUSTOM_COMMAND(TARGET eve_hal POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/dependencies/ft4222/dll/i386/LibFT4222.dll
      ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/LibFT4222.dll)
    ADD_CUSTOM_COMMAND(TARGET eve_hal POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_SOURCE_DIR}/dependencies/ftd2xx/i386/ftd2xx.dll
      ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/ftd2xx.dll)
  ENDIF()
ENDIF()
