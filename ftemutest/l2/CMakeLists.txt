
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_l2
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_l2
	ftemutest
	)
