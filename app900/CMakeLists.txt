
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftapp900
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftapp900
	libft900emu
	)

