copy ..\build_vs2017_x64\bin\Release\bt8xxemu.dll bt8xxemu.dll
copy ..\build_vs2017_x64\bin\Release\mx25lemu.dll mx25lemu.dll
copy ..\build_vs2017_x64\bin\Release\fteditor.exe fteditor.exe
rmdir /s /q export_scripts
mkdir export_scripts
xcopy ..\fteditor\export_scripts export_scripts /sy
rem rmdir /s /q Lib
mkdir Lib
xcopy C:\2018q3-external-vs2017-x64\python27\Lib Lib /syd
copy C:\2018q3-external-vs2017-x64\python27\bin\python27.dll python27.dll
copy ..\fteditor\img_cvt.py img_cvt.py
rem copy ..\fteditor\raw_cvt.py raw_cvt.py
copy ..\fteditor\png.py png.py
copy ..\fteditor\pngp2pa.py pngp2pa.py
del /S *.pyc
del /S *.pyo
rmdir /s /q firmware
mkdir firmware
xcopy ..\fteditor\firmware firmware /sy
copy ..\fteditor\ftlib\ftd2xx.dll ftd2xx.dll
copy ..\fteditor\ftlib\libMPSSE.dll libMPSSE.dll
copy ..\bin\pngquant.exe pngquant.exe
copy C:\2018q3-external-vs2017-x64\qtbase\bin\Qt5Core.dll Qt5Core.dll
copy C:\2018q3-external-vs2017-x64\qtbase\bin\Qt5Gui.dll Qt5Gui.dll
copy C:\2018q3-external-vs2017-x64\qtbase\bin\Qt5Widgets.dll Qt5Widgets.dll
copy C:\2018q3-external-vs2017-x64\qtbase\bin\libGLESv2.dll libGLESv2.dll
copy C:\2018q3-external-vs2017-x64\qtbase\bin\libEGL.dll libEGL.dll
rmdir /s /q platforms
mkdir platforms
cd platforms
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\platforms\qwindows.dll qwindows.dll
cd ..
rmdir /s /q styles
mkdir styles
cd styles
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\styles\qwindowsvistastyle.dll qwindowsvistastyle.dll
cd ..
rmdir /s /q imageformats
mkdir imageformats
cd imageformats
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\imageformats\qjpeg.dll qjpeg.dll
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\imageformats\qgif.dll qgif.dll
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\imageformats\qtga.dll qtga.dll
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\imageformats\qwbmp.dll qwbmp.dll
copy C:\2018q3-external-vs2017-x64\qtbase\plugins\imageformats\qtiff.dll qtiff.dll
cd ..
copy C:\2018q3-external-vs2017-x64\freetype\bin\freetype.dll freetype.dll
copy C:\2018q3-external-vs2017-x64\libpng\bin\libpng16.dll libpng16.dll
copy C:\2018q3-external-vs2017-x64\zlib\bin\zlib.dll zlib.dll

"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode fteditor.exe
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode bt8xxemu.dll
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode mx25lemu.dll
