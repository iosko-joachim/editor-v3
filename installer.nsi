!include "MUI2.nsh"

Name "Editor v3"
OutFile "editor_v3_setup.exe"
InstallDir "$PROGRAMFILES64\Editor v3"
RequestExecutionLevel admin

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "German"

Section "Install"
    SetOutPath "$INSTDIR"
    File /r "deploy\*.*"

    ; Start menu shortcut
    CreateDirectory "$SMPROGRAMS\Editor v3"
    CreateShortcut "$SMPROGRAMS\Editor v3\Editor v3.lnk" "$INSTDIR\editor_v3.exe"
    CreateShortcut "$SMPROGRAMS\Editor v3\Deinstallieren.lnk" "$INSTDIR\uninstall.exe"

    ; Desktop shortcut
    CreateShortcut "$DESKTOP\Editor v3.lnk" "$INSTDIR\editor_v3.exe"

    ; Uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Registry for Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3" "DisplayName" "Editor v3"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3" "Publisher" "Editor v3"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3" "NoRepair" 1
SectionEnd

Section "Uninstall"
    RMDir /r "$INSTDIR"
    RMDir /r "$SMPROGRAMS\Editor v3"
    Delete "$DESKTOP\Editor v3.lnk"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\EditorV3"
SectionEnd
