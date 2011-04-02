@echo off
REM Run this from the Visual Studio command prompt, this will recompile all binaries with a version number
set buildparams=/t:Rebuild /verbosity:minimal /nologo

set file=ohips_cpp.sln
msbuild %buildparams% %file% /p:Configuration=Debug /p:Platform=Win32
msbuild %buildparams% %file% /p:Configuration=Debug /p:Platform=x64
msbuild %buildparams% %file% /p:Configuration=Release /p:Platform=Win32
msbuild %buildparams% %file% /p:Configuration=Release /p:Platform=x64

set file=ohips_cs.sln
msbuild %buildparams% %file% /p:Configuration=Debug
msbuild %buildparams% %file% /p:Configuration=Release
