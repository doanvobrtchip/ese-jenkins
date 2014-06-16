
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h avr/*.h)

ADD_LIBRARY(libftarduino
	${SRCS}
	${HDRS}
	)

target_link_libraries(libftarduino
	libft800emu
	)
