
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(btdump2
	${SRCS}
	${HDRS}
)

TARGET_LINK_LIBRARIES(btdump2
	bt8xxemu
)
