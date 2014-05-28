@echo off
set SORA_ROOT=%CD%
start %comspec% /k %SORA_ROOT%\build\setenv.bat %WINDDK_ROOT% chk x64 win7