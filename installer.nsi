!ifndef APP_VERSION
  !define APP_VERSION "dev"
!endif

; Override build output directory if needed: makensis /DBUILD_DIR=... installer.nsi
!ifndef BUILD_DIR
  !define BUILD_DIR "DesktopPet.Windows\build"
!endif

!define APP_NAME "Desktop Pet"
!define APP_EXE "DesktopPet.exe"
!define PUBLISHER "bssm-oss"
!define REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "DesktopPet-Windows-Setup.exe"
InstallDir "$LOCALAPPDATA\DesktopPet"
RequestExecutionLevel user
SetCompressor lzma

!include "MUI2.nsh"

!define MUI_ABORTWARNING
!define MUI_ICON "DesktopPet.Windows\app.ico"
!define MUI_UNICON "DesktopPet.Windows\app.ico"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"

    File "${BUILD_DIR}\${APP_EXE}"

    WriteRegStr HKCU "Software\${APP_NAME}" "InstallDir" "$INSTDIR"

    WriteRegStr HKCU "${REG_KEY}" "DisplayName"     "${APP_NAME}"
    WriteRegStr HKCU "${REG_KEY}" "DisplayVersion"  "${APP_VERSION}"
    WriteRegStr HKCU "${REG_KEY}" "Publisher"       "${PUBLISHER}"
    WriteRegStr HKCU "${REG_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKCU "${REG_KEY}" "DisplayIcon"     "$INSTDIR\${APP_EXE}"
    WriteRegDWORD HKCU "${REG_KEY}" "NoModify"      1
    WriteRegDWORD HKCU "${REG_KEY}" "NoRepair"      1

    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"   "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir  "$INSTDIR"

    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
    RMDir  "$SMPROGRAMS\${APP_NAME}"

    DeleteRegKey HKCU "Software\${APP_NAME}"
    DeleteRegKey HKCU "${REG_KEY}"
SectionEnd
