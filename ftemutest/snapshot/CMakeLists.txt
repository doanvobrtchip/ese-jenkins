
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_snapshot
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_snapshot
	ftemutest
	)
