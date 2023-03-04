@echo === SET environment for cmake and nmake =========

@set "PATH=%PATH%;%VSINSTALLDIR%Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSINSTALLDIR%Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

@for /D %%i in ("%VSINSTALLDIR%VC\Tools\MSVC\*") do @set __CVTOOLS=%%i


@if EXIST "%__CVTOOLS%\bin\Hostx86\x86" (
    @set "PATH=%PATH%;%__CVTOOLS%\bin\Hostx86\x86;"
) else if EXIST "%__CVTOOLS%\bin\Hostx86\x64"  (
    @set "PATH=%PATH%;%__CVTOOLS%\bin\Hostx86\x64;"
) else if EXIST "%__CVTOOLS%\bin\Hostx64\x86"  (
    @set "PATH=%PATH%;%__CVTOOLS%\bin\Hostx64\x86;"
) else if EXIST "%__CVTOOLS%\bin\Hostx64\x64"  (
    @set "PATH=%PATH%;%__CVTOOLS%\bin\Hostx64\x64;"
) else (
    @call "%VSINSTALLDIR%Common7\Tools\VsDevCmd.bat
)


