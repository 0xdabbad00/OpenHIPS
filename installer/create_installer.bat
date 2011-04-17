SET WIXDIR="C:\Program Files (x86)\Windows Installer XML v3.5\bin"
SET OPTIONS=-ext WixUtilExtension -ext WiXNetFxExtension

del *.msi installer.wix*
%WIXDIR%\candle.exe %OPTIONS% installer.wxs 
%WIXDIR%\light.exe %OPTIONS% installer.wixobj
move installer.msi openhips-setup.msi
del installer.wix*
%WIXDIR%\candle.exe -dIsDebug="true" %OPTIONS% installer.wxs 
%WIXDIR%\light.exe %OPTIONS% installer.wixobj
move installer.msi openhips-setup_debug.msi