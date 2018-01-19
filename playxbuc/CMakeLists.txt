
FILE(GLOB SRCS ../playxbu/*.cpp ../bt8xxemuc/bt8xxemuc.c)
FILE(GLOB HDRS ../playxbu/*.h)

ADD_EXECUTABLE(btplayxbuc
	${SRCS}
	${HDRS}
	)

ADD_DEFINITIONS(-DBT8XXEMU_REMOTE)
