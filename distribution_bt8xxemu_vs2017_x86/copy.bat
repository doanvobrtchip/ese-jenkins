copy ..\build_vs2017_x86\bin\Release\bt8xxemu.dll bt8xxemu.dll
copy ..\build_vs2017_x86\lib\Release\bt8xxemu.lib bt8xxemu.lib
copy ..\build_vs2017_x86\bin\Release\mx25lemu.dll mx25lemu.dll
copy ..\build_vs2017_x86\bin\Release\bt8xxemus.exe bt8xxemus.exe

copy ..\build_vs2017_x86\bin\Release\ft8xxemu.dll ft8xxemu.dll
copy ..\build_vs2017_x86\lib\Release\ft8xxemu.lib ft8xxemu.lib

copy ..\bt8xxemu\bt8xxemu.h bt8xxemu.h
copy ..\bt8xxemu\bt8xxemu_diag.h bt8xxemu_diag.h
copy ..\bt8xxemu\bt8xxemu_flash.h bt8xxemu_flash.h
copy ..\bt8xxemu\bt8xxemu_inttypes.h bt8xxemu_inttypes.h
copy ..\bt8xxemuc\bt8xxemuc.c bt8xxemuc.c

copy ..\ft8xxemu\ft8xxemu.h ft8xxemu.h
copy ..\ft8xxemu\ft8xxemu_diag.h ft8xxemu_diag.h
copy ..\ft8xxemu\ft8xxemu_inttypes.h ft8xxemu_inttypes.h

"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode bt8xxemu.dll
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode mx25lemu.dll
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode bt8xxemus.exe

"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 028eb5ba2732d21fb599aa88d9e0c26ac76b4045 /t http://timestamp.comodoca.com/authenticode ft8xxemu.dll
