
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_fifoimage
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_fifoimage
	ftemutest
	)
