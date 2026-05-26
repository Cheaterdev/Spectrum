git submodule sync
git submodule update Sharpmake
dotnet build ./Sharpmake/Sharpmake.sln --artifacts-path ./Sharpmake/output /p:Configuration=Release /p:Platform="Any CPU"
call generate_project.bat
