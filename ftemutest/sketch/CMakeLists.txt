
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_sketch
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_sketch
	ftemutest
	)
