@echo off
setlocal
set MSYS=winsymlinks:nativestrict
C:\\msys64\\msys2_shell.cmd -defterm -no-start -mingw64 -full-path -here %*
