del build\*.exe
msbuild /t:Clean /p:Configuration=Release /p:Platform=x64 jucer\Builds\VisualStudio2017\Versicap.sln || exit /b
