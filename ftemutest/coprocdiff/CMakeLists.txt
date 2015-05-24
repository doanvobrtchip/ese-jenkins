
FILE(GLOB SRCS *.cpp ../../fteditor/constant_*.cpp ../../fteditor/dl_parser*.cpp)
FILE(GLOB HDRS *.h ../../fteditor/constant_*.h ../../fteditor/dl_parser*.h)

CMAKE_POLICY(SET CMP0020 NEW)

# Find the Qt5Gui library
FIND_PACKAGE(Qt5Gui)

ADD_EXECUTABLE(ftemutest_coprocdiff
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_coprocdiff
	Qt5::Gui
	ftemutest
	)

qt5_use_modules(ftemutest_coprocdiff Gui)
