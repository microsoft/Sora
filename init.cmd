@echo off

for /F "eol= delims=~" %%d in ('CD') do set CurrentDirectory=%%d

:InputDDKRoot
echo Please specify where WinDDK is installed
echo For example: [C:\WinDDK\6001.18002]
SET /P WINDDK_INSTROOT=WinDDK=:
if exist %WINDDK_INSTROOT% (
    if not exist %WINDDK_INSTROOT%\bin\setenv.bat (
        echo The folder [%WINDDK_INSTROOT%\bin] doesn't exist.
        goto InputDDKRoot
    )
) else (
    echo The folder [%WINDDK_INSTROOT%] doesn't exist.
    goto InputDDKRoot
)

set "args= /k set SORA_ROOT=%CurrentDirectory%&%CurrentDirectory%\build\setenv.bat %WINDDK_INSTROOT% chk x86 WXP"
"%CurrentDirectory%\build\shortcut.exe" /p:"cmd.exe" /n:"Sora WXP x86 Chk Build.lnk" /a:"%args%"

set "args= /k set SORA_ROOT=%CurrentDirectory%&%CurrentDirectory%\build\setenv.bat %WINDDK_INSTROOT% fre x86 WXP"
"%CurrentDirectory%\build\shortcut.exe" /p:"cmd.exe" /n:"Sora WXP x86 Fre Build.lnk" /a:"%args%"

set "args= /k set SORA_ROOT=%CurrentDirectory%&%CurrentDirectory%\build\setenv.bat %WINDDK_INSTROOT% chk x64 WIN7"
"%CurrentDirectory%\build\shortcut.exe" /p:"cmd.exe" /n:"Sora WIN7 x64 Chk Build.lnk" /a:"%args%"

set "args= /k set SORA_ROOT=%CurrentDirectory%&%CurrentDirectory%\build\setenv.bat %WINDDK_INSTROOT% fre x64 WIN7"
"%CurrentDirectory%\build\shortcut.exe" /p:"cmd.exe" /n:"Sora WIN7 x64 Fre Build.lnk" /a:"%args%"

echo Sora build environment is initialized.
