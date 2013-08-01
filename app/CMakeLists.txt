
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftapp
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftapp
	libft800emu
	libftarduino
	)
