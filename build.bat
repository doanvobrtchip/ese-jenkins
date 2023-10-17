::Check Qt dir
IF "%QTDIR64%"=="" (SET QT_DIR="C:\Qt\LatestVersion\6.4.0\msvc2019_64") ELSE (SET QT_DIR=%QTDIR64%)
IF "%CERDIR%"=="" (SET CER_DIR="%cd%\bridgetek_pte_ltd.p12") ELSE (SET CER_DIR=%CERDIR%)

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

SET signToolPath="%PROGRAMFILES(X86)%\Windows Kits\10\App Certification Kit\signtool.exe"
%signToolPath% sign /f %CER_DIR% /p Brtsw2023 /t http://timestamp.digicert.com "%cd%\build\bin\Release\fteditor.exe"
%signToolPath% sign /f %CER_DIR% /p Brtsw2023 /t http://timestamp.digicert.com "%cd%\build\bin\Release\mx25lemu.dll"
%signToolPath% sign /f %CER_DIR% /p Brtsw2023 /t http://timestamp.digicert.com "%cd%\build\bin\Release\bt8xxemu.dll"
%signToolPath% sign /f %CER_DIR% /p Brtsw2023 /t http://timestamp.digicert.com "%cd%\build\bin\Release\eve_hal.dll"
\build\bin\Release
::Package
iscc deployment/ese/setup_v2.iss
::Create installer and zip 
FOR %%I IN (%cd%\deployment\ese\Output\*.exe) DO (
    ::Sign certificate
    %signToolPath% sign /f %CER_DIR% /p Brtsw2023 /t http://timestamp.digicert.com "%cd%\deployment\ese\Output\%%~nI.exe"
    ::Compress the package
    7Z a "%cd%\deployment\ese\Output\%%~nI.zip" "%cd%\deployment\ese\Output\%%~nI.exe"
)