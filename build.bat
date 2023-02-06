::Check Qt dir
IF "%QTDIR64%"=="" (SET QT_DIR="C:\Qt\LatestVersion\6.4.0\msvc2019_64") ELSE (SET QT_DIR=%QTDIR64%)

RMDIR /S /Q build
CMAKE -Bbuild/ ^
    -S. ^
    -G "Visual Studio 16 2019" ^
    -DCMAKE_PREFIX_PATH=%QT_DIR%^
    -DWITH_QT=TRUE ^
    -DWITH_WINDEPLOYQT=TRUE ^
    -DWITH_PYTHON=TRUE ^
    -DWITH_FREETYPE=TRUE ^
    -DWITH_EMUTEST=FALSE ^
    -DFREETYPE_INCLUDE_DIRS:STRING=%cd%\fteditor\dependencies\freetype\include ^
    -DFREETYPE_LIBRARY:STRING=%cd%\fteditor\dependencies\freetype\freetype.lib"
CMAKE --build build --config Release

::Create installer and zip 
iscc deployment/ese/setup_v2.iss
FOR %%I IN (%cd%\deployment\ese\Output\*.exe) DO 7Z a "%cd%\deployment\ese\Output\%%~nI.zip" "%cd%\deployment\ese\Output\%%~nI.exe"