@echo off 
:: Collect files to create installer package for ESE

set build-dir=D:\ESE\build-x64\bin\Release
set setup-dir=setup
set qt-bin=C:\Qt5\5.15.2\msvc2019_64\bin

rmdir /s /q %setup-dir%
mkdir %setup-dir%

device_sync
EVE_Hal_Library
Examples
export_scripts
export_template
firmware
Manual
untitled

xcopy /siy ..\fteditor\device_sync %setup-dir%\device_sync
xcopy /siy ..\fteditor\EVE_Hal_Library %setup-dir%\EVE_Hal_Library
xcopy /siy ..\fteditor\Examples %setup-dir%\Examples
xcopy /siy ..\fteditor\export_scripts %setup-dir%\export_scripts
xcopy /siy ..\fteditor\export_template %setup-dir%\export_template
xcopy /siy ..\fteditor\firmware %setup-dir%\firmware
xcopy /siy ..\fteditor\untitled %setup-dir%\untitled

xcopy /siy %build-dir%\Lib %setup-dir%\Lib 


:: copy all python files
xcopy /y ..\fteditor\*.py %setup-dir%
del /S %setup-dir%\*.pyc
del /S %setup-dir%\*.pyo

:: copy all dll files
xcopy /y %build-dir%\*.dll %setup-dir%

:: copy all exe files
xcopy /y %build-dir%\*.exe %setup-dir%

:: copy icon
xcopy /y ..\fteditor\icons\eve-puzzle-64.ico %setup-dir%

::copy config
xcopy /y ..\fteditor\config.json %setup-dir%

xcopy /y ..\deployment\ese\setup.iss %setup-dir%
xcopy /y ..\deployment\ese\prerelease.txt %setup-dir%

:: rename manual file
xcopy ..\fteditor\Manual\*.pdf %setup-dir%\Manual\

:: run Qt deployment
%qt-bin%\windeployqt --release --force --no-translations %setup-dir%\fteditor.exe

pause