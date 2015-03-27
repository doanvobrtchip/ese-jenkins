
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_playvideo
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_playvideo
	ftemutest
	)
