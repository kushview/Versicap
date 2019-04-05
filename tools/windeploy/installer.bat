mkdir build
makensis tools\windeploy\x64.nsi || exit /b
copy build\versicap-win64-1.0.0.exe build\versicap-win64-latest.exe || exit /b
signtool sign /f c:\SDKs\KushviewCert.p12 /p ***REMOVED*** /tr http://timestamp.comodoca.com build\versicap-win*.exe || exit /b
