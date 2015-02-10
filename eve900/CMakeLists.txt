
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(fteve900
	${SRCS}
	${HDRS}
	)

target_link_libraries(fteve900
	ft8xxemu_api
	ft900emu
	)

