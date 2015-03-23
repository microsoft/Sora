copy %sora_root%\setup\CoreSDK\x64\*.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64
copy %sora_root%\setup\CoreSDK\x64\*.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64
inf2cat /driver:%SORA_ROOT%\target\ke\chk_win7_amd64\amd64 /os:7_X64
inf2cat /driver:%SORA_ROOT%\target\ke\fre_win7_amd64\amd64 /os:7_X64

Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64\PCIE.sys
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64\HWTest.sys
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64\SDRMiniport.sys
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64\PCIE.sys
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64\HWTest.sys
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64\SDRMiniport.sys

Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64\PCIE.cat
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64\HWTest.cat
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\chk_win7_amd64\amd64\SDR.cat
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64\PCIE.cat
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64\HWTest.cat
Signtool sign /v /s PrivateCertStore /n "Microsoft Research Asia" /t http://timestamp.verisign.com/scripts/timestamp.dll %SORA_ROOT%\target\ke\fre_win7_amd64\amd64\SDR.cat