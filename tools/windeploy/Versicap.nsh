; Versicap NSIS Install Script
; Michael Fisher <mfisher@kushview.net>

;--------------------------------
; Includes
!include 'LogicLib.nsh'
!include "MUI2.nsh"
!include x64.nsh

;--------------------------------
; Defines
!define TOPDIR "..\.."
!define INSTALLER_ICON "${TOPDIR}\data\VersicapIcon.ico"
!define UNINSTALLER "Uninstall ${VERSICAP_APP_NAME}.exe"

;--------------------------------
; Branding
!define MUI_PRODUCT "${VERSICAP_APP_NAME}"
!define MUI_FILE "savefile"
!define MUI_VERSION "${VERSICAP_VERSION}"
!define MUI_BRANDINGTEXT "${MUI_PRODUCT} v${MUI_VERSION}"

!ifdef INNER
  !echo "Inner invocation"                  ; just to see what's going on
  OutFile "$%TEMP%\tempinstaller.exe"       ; not really important where this is
  SetCompress off                           ; for speed
!else
  !echo "Outer invocation"
 
  ; Call makensis again, defining INNER.  This writes an installer for us which, when
  ; it is invoked, will just write the uninstaller to some location, and then exit.
  ; Be sure to substitute the name of this script here.
 
  !system "$\"${NSISDIR}\makensis$\" /DINNER ${VERSICAP_SCRIPT}" = 0
 
  ; So now run that installer we just created as %TEMP%\tempinstaller.exe.  Since it
  ; calls quit the return value isn't zero.
 
  !system "$%TEMP%\tempinstaller.exe" = 2
 
  ; That will have written an uninstaller binary for us.  Now we sign it with your
  ; favorite code signing tool.
 
  ; !system "SIGNCODE <signing options> $%TEMP%\uninstall.exe" = 0
  !system "signtool sign /f c:\SDKs\KushviewCert.p12 /p ***REMOVED*** /tr http://timestamp.comodoca.com $\"$%TEMP%\${UNINSTALLER}$\"" = 0
  ; Good.  Now we can carry on writing the real installer.
 
  OutFile "${TOPDIR}\build\${PACKAGE_BASENAME}-${VERSICAP_ARCH}-${MUI_VERSION}.exe"
  SetCompressor /SOLID lzma
!endif

Function .onInit
!ifdef INNER
  ; If INNER is defined, then we aren't supposed to do anything except write out
  ; the installer.  This is better than processing a command line option as it means
  ; this entire code path is not present in the final (real) installer.
  WriteUninstaller "$%TEMP%\${UNINSTALLER}"
  Quit  ; just bail out quickly when running the "inner" installer
!endif
  UserInfo::GetAccountType
  pop $0
  ${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox mb_iconstop "Administrator rights required!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}
  StrCpy $InstDir "${VERSICAP_INSTDIR}"
FunctionEnd

;--------------------------------
; Setup Branding, defined in Common.nsh
CRCCheck On
BrandingText "${MUI_BRANDINGTEXT}"

;--------------------------------
; General
;Name and file
Name "${VERSICAP_APP_NAME}"

;Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
;Interface Settings
!define MUI_ABORTWARNING

;--------------------------------
;Icons To Use
; !define MUI_ICON ${INSTALLER_ICON}
; !define MUI_UNICON ${INSTALLER_ICON}

;--------------------------------
;Pages
; !define MUI_FINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_PAGE_WELCOME
; !insertmacro MUI_PAGE_LICENSE "${TOPDIR}\data\EULA.txt"

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${VERSICAP_APP_NAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${VERSICAP_APP_NAME}"

!define MUI_FINISHPAGE_TEXT_REBOOT "The setup needs to reboot"
!define MUI_FINISHPAGE_TEXT_REBOOTNOW "Reboot now"
!define MUI_FINISHPAGE_TEXT_REBOOTLATER "Reboot later"

!insertmacro MUI_PAGE_FINISH

!ifdef INNER
 !insertmacro MUI_UNPAGE_CONFIRM
 !insertmacro MUI_UNPAGE_INSTFILES
!endif

;--------------------------------
;Languages
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; MUI System
; !insertmacro MUI_SYSTEM

;--------------------------------
; Installer Sections
Section "${VERSICAP_APP_NAME} Application" SecApp
    ; Read Only (mandatory)
    SectionIn RO

    SetOutPath "$INSTDIR"
    File "${VERSICAP_EXE_FILE}"
!ifndef INNER
    ; this packages the signed uninstaller
    File "$%TEMP%\${UNINSTALLER}"
!endif
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${VERSICAP_APP_NAME}" \
                     "DisplayName" "${VERSICAP_APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${VERSICAP_APP_NAME}" \
                     "Publisher" "Kushview, LLC"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${VERSICAP_APP_NAME}" \
                     "UninstallString" "$\"$INSTDIR\${UNINSTALLER}$\""
SectionEnd

;--------------------------------
; Desktp Shortcut Section
Section "Desktop Shortcut" secDesktop
    SetShellVarContext current
    CreateShortCut "$DESKTOP\${VERSICAP_APP_NAME}.lnk" "$INSTDIR\${VERSICAP_APP_NAME}.exe"
SectionEnd

;--------------------------------
; Start Menu Section
Section "Start Menu" secStartMenu
    SetShellVarContext current
    CreateDirectory "$SMPROGRAMS\Kushview\Versicap"
    CreateShortCut "$SMPROGRAMS\Kushview\Versicap\${VERSICAP_APP_NAME}.lnk" "$INSTDIR\${VERSICAP_APP_NAME}.exe"
    CreateShortCut "$SMPROGRAMS\Kushview\Versicap\Uninstall ${VERSICAP_APP_NAME}.lnk" "$INSTDIR\${UNINSTALLER}"
SectionEnd

!ifdef INNER
;--------------------------------
; Uninstaller Section
Section "Uninstall"
    SetShellVarContext current
    Delete "$DESKTOP\${VERSICAP_APP_NAME}.lnk"
    Delete "$INSTDIR\${UNINSTALLER}"
    Delete "$INSTDIR\${VERSICAP_APP_NAME}.exe"
   
    RmDir /r "$SMPROGRAMS\Kushview\Versicap"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${VERSICAP_APP_NAME}"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Run\${VERSICAP_APP_NAME}"
SectionEnd
!endif
