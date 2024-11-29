@echo off

@echo Copy Release to d:\opt\bin
copy llpeak-ms\x64\Release\llpeak.exe d:\opt\bin\llpeak.exe


@echo
@echo Compare md5 hash
cmp -h llpeak-ms\x64\Release\llpeak.exe d:\opt\bin\llpeak.exe
ld -a d:\opt\bin\llpeak.exe

@echo
@echo List all llpeak.exe
ld -r -F=llpeak.exe bin d:\opt\bin
