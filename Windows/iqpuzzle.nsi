;iQPuzzle NSIS installer script

  !define APPNAME "iQPuzzle"
  !define DESCRIPTION "IQ challenging pentomino puzzle"
  !define VERSIONMAJOR 1
  !define VERSIONMINOR 3
  !define VERSIONPATCH 1
  !define APPVERSION "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONPATCH}.0"
  !define ABOUTURL "https://elth0r0.github.io/iqpuzzle/"

  !include "LogicLib.nsh"
  !include "MUI2.nsh"
  !include "FileFunc.nsh"

  SetCompressor /SOLID /FINAL lzma

  Name "${APPNAME}"
  OutFile "${APPNAME}_Installer.exe"
  InstallDir "$PROGRAMFILES\${APPNAME}"
  InstallDirRegKey HKLM "Software\${APPNAME}" ""
  !define MUI_ICON "iqpuzzle.ico"
  RequestExecutionLevel admin

  VIProductVersion ${APPVERSION}
  VIAddVersionKey ProductName "${APPNAME}"
  VIAddVersionKey LegalCopyright "Thorsten Roth"
  VIAddVersionKey FileDescription "${DESCRIPTION}"
  VIAddVersionKey FileVersion "${APPVERSION}"
  VIAddVersionKey ProductVersion "${APPVERSION}"
  VIAddVersionKey InternalName "${APPNAME}"
  BrandingText "${APPNAME} - ${APPVERSION}"

  Var StartMenuFolder
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPNAME}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !define MUI_ABORTWARNING
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer section

Section

  SetOutPath "$INSTDIR"

  ;Add all files from folder iQPuzzle into installer
  File /r ${APPNAME}\*.*

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  SetShellVarContext all
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk" "$INSTDIR\iQPuzzle.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
  SetShellVarContext current

  WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                   "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                   "Publisher" "Thorsten Roth"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                   "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$\"$INSTDIR\${APPNAME}.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                   "URLInfoAbout" "$\"${ABOUTURL}$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                   "DisplayVersion" "${APPVERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                     "VersionMajor" ${VERSIONMAJOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                     "VersionMinor" ${VERSIONMINOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                     "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                     "NoRepair" 1
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                     "EstimatedSize" "$0"

SectionEnd

;--------------------------------
;Uninstaller section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"

  SetShellVarContext all
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  SetShellVarContext current

  DeleteRegKey HKLM "Software\${APPNAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

SectionEnd

;--------------------------------

Function .onInit
  UserInfo::GetAccountType
  pop $0
  ${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox mb_iconstop "Administrator rights required for installation!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}
FunctionEnd
