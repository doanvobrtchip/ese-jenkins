
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_touchtfr
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_touchtfr
	ftemutest
	)
