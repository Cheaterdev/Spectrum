set VS_TYPE=prerelease
call Sharpmake/bootstrap.bat
call install_agility_sdk.bat
call generate_project.bat
vcpkg integrate install
