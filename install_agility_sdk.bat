rmdir AgilitySDK
mkdir AgilitySDK
powershell -command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.610.0 -OutFile agility.zip"
powershell -command "Expand-Archive agility.zip -DestinationPath AgilitySDK"
powershell -command "rm agility.zip"

rmdir PIX
mkdir PIX
powershell -command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001 -OutFile pix.zip"
powershell -command "Expand-Archive pix.zip -DestinationPath PIX"
powershell -command "rm pix.zip"