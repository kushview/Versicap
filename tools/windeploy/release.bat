call tools\windeploy\sync-jucer.bat || exit /b
call tools\windeploy\clean.bat || exit /b
call tools\windeploy\build.bat || exit /b
call tools\windeploy\installer.bat || exit /b
