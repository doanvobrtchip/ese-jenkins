
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_htf
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_htf
	ftemutest
	)
