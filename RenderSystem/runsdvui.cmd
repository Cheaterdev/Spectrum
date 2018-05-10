cd /d "H:\Spectrum\RenderSystem" &msbuild "RenderSystem.vcxproj" /t:sdvViewer /p:configuration="Release" /p:platform=Win32
exit %errorlevel% 