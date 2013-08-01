
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftloaddump
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftloaddump
	libft800emu
	)
