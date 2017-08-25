
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ft900eve
	${SRCS}
	${HDRS}
	)

target_link_libraries(ft900eve
	bt8xxemu
	ft900emu
	)

