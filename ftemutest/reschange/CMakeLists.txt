
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_reschange
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_reschange
	ftemutest
	)
