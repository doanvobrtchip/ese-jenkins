
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_cmdb
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_cmdb
	ftemutest
	)
