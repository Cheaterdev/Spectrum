mkdir AgilitySDK
cd AgilitySDK
powershell -command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.4.10 -OutFile agility.zip"
powershell -command "Expand-Archive agility.zip -DestinationPath d3d"
powershell -command "rm .\agility.zip"