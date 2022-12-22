RMDIR /S /Q build
CMAKE -Bbuild/ ^
    -S. ^
    -G "Visual Studio 16 2019" ^
    -DCMAKE_PREFIX_PATH=%QTDIR64%^
    -DWITH_QT=TRUE ^
    -DWITH_PYTHON=TRUE ^
    -DWITH_FREETYPE=TRUE ^
    -DWITH_EMUTEST=FALSE ^
    -DFREETYPE_INCLUDE_DIRS:STRING=%cd%\freetype\include ^
    -DFREETYPE_LIBRARY:STRING=%cd%\freetype\build\Release\freetype.lib ^
    -DPNG_LIBRARY:STRING="./GnuWin32/lib/libpng.lib" ^
    -DPNG_PNG_INCLUDE_DIR:STRING="./GnuWin32/include" ^
    -DZLIB_LIBRARY:STRING="./zlib-1.2.3-bin/bin" ^
    -DZLIB_INCLUDE_DIR:STRING="./zlib/build/Release/zlib.lib"
CMAKE --build build --config Release

::Create installer and zip 
iscc deployment/ese/setup_v2.iss
FOR %%I IN (%cd%\deployment\ese\Output\*.exe) DO 7Z a "%cd%\deployment\ese\Output\%%~nI.zip" "%cd%\deployment\ese\Output\%%~nI.exe"