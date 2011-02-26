del *.msi installer.wix*
copy ..\bin\Debug\*.dll ..\bin\Release\.
"C:\Program Files (x86)\Windows Installer XML v3.5\bin\candle.exe" installer.wxs
"C:\Program Files (x86)\Windows Installer XML v3.5\bin\light.exe" installer.wixobj
move installer.msi openhips-setup.msi