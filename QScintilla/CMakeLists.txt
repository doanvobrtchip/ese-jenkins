# Find the QtWidgets library
FIND_PACKAGE(Qt6Widgets)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB _SRCS 
  *.cpp 
  src/*.cpp 
  scintilla/src/*.cpp 
  scintilla/lexers/*.cpp 
  scintilla/lexlib/*.cpp
)

file(GLOB _IMPL_HDRS 
  *.h 
  scintilla/include/*.h 
  src/QSci/*.h
)

set(TARGET_NAME "QScintilla")


INCLUDE_DIRECTORIES(
  scintilla/include
  scintilla/lexlib
  scintilla/src
  src
  )

ADD_LIBRARY(${TARGET_NAME} SHARED
  ${_SRCS}
  ${_IMPL_HDRS}
  )
  
TARGET_LINK_LIBRARIES(${TARGET_NAME} 
  Qt6::Widgets 
)

ADD_DEFINITIONS(-DQSCINTILLA_MAKE_DLL -DSCINTILLA_QT -DSCI_LEXER -D_CRT_SECURE_NO_WARNINGS)  

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES FOLDER "[0] 3rd Party")

