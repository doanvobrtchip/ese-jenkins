
FILE(GLOB SRCS *.cpp ../../fteditor/constant_*.cpp ../../fteditor/dl_parser*.cpp)
FILE(GLOB HDRS *.h ../../fteditor/constant_*.h ../../fteditor/dl_parser*.h)

CMAKE_POLICY(SET CMP0020 NEW)

# Find the Qt5Gui library
find_package(Qt6 QUIET COMPONENTS Gui)
if(NOT Qt6_FOUND)
  find_package(Qt5 5.15 REQUIRED COMPONENTS Gui)
endif()

ADD_EXECUTABLE(ftemutest_coprocdiff
	${SRCS}
	${HDRS}
	)

target_link_libraries(ftemutest_coprocdiff
	Qt::Gui
	ftemutest
	)
