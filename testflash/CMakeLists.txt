
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(bttestflash
	${SRCS}
	${HDRS}
)

TARGET_LINK_LIBRARIES(bttestflash
	bt8xxemu
)
