
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_lineclocks
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_lineclocks
	ftemutest
	)
