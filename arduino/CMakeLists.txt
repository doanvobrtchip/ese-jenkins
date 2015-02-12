
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h avr/*.h)

ADD_LIBRARY(ftarduino
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftarduino
	ft8xxemu
	)
