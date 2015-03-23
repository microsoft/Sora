pushd %SORA_ROOT%\kernel\core\src\usora
build -cZMg
cd %SORA_ROOT%\kernel\util
build -cZMg
cd %SORA_ROOT%\kernel\bb\dot11b\ulib
build -cZMg
cd %SORA_ROOT%\kernel\bb
build -cZMg
popd