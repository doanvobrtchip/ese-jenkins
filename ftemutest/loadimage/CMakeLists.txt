
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_loadimage
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_loadimage
	ftemutest
	)
