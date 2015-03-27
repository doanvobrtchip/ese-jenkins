
FILE(GLOB SRCS *.cpp)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_paletted
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_paletted
	ftemutest
	)
