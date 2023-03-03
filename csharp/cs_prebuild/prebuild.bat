
@echo === RUN prebuild.bat =======================

@SET scriptpath=%~dp0

@pushd "%scriptpath%"

call setenv.bat


@cd "%TARGETDIR%"

@if not EXIST "%TARGETFILEWITHPATH%" (
    cmake -G "NMake Makefiles" "%scriptpath%"
)
    
nmake

@popd
