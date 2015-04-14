
FILE(GLOB SRCS *.cpp *.c)
FILE(GLOB HDRS *.h)

ADD_EXECUTABLE(ftemutest_sampleapp
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_sampleapp
	ftemutest
	)
