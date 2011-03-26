del *.msi installer.wix*
"C:\Program Files (x86)\Windows Installer XML v3.5\bin\candle.exe" -ext WixUtilExtension installer.wxs 
"C:\Program Files (x86)\Windows Installer XML v3.5\bin\light.exe" -ext WixUtilExtension installer.wixobj
move installer.msi openhips-setup.msi
del installer.wix*
"C:\Program Files (x86)\Windows Installer XML v3.5\bin\candle.exe" -dIsDebug="true" -ext WixUtilExtension installer.wxs 
"C:\Program Files (x86)\Windows Installer XML v3.5\bin\light.exe" -ext WixUtilExtension installer.wixobj
move installer.msi openhips-setup_debug.msi