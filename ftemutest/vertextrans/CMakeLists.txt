
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_vertextrans
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_vertextrans
	ftemutest
	)
