
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_gradient
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_gradient
	ftemutest
	)
