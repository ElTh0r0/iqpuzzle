;iQPuzzle NSIS installer script

  !define APPNAME "iQPuzzle"
  !define DESCRIPTION "A challenging pentomino puzzle"
  !define VERSIONMAJOR 1
  !define VERSIONMINOR 4
  !define VERSIONPATCH 2
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
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

  !define MUI_LANGDLL_ALLLANGUAGES
  !define MUI_ABORTWARNING
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
  !insertmacro MUI_LANGUAGE "English"  ;Default
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SpanishInternational"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "NorwegianNynorsk"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "SerbianLatin"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
  !insertmacro MUI_LANGUAGE "Mongolian"
  !insertmacro MUI_LANGUAGE "Luxembourgish"
  !insertmacro MUI_LANGUAGE "Albanian"
  !insertmacro MUI_LANGUAGE "Breton"
  !insertmacro MUI_LANGUAGE "Belarusian"
  !insertmacro MUI_LANGUAGE "Icelandic"
  !insertmacro MUI_LANGUAGE "Malay"
  !insertmacro MUI_LANGUAGE "Bosnian"
  !insertmacro MUI_LANGUAGE "Kurdish"
  !insertmacro MUI_LANGUAGE "Irish"
  !insertmacro MUI_LANGUAGE "Uzbek"
  !insertmacro MUI_LANGUAGE "Galician"
  !insertmacro MUI_LANGUAGE "Afrikaans"
  !insertmacro MUI_LANGUAGE "Catalan"
  !insertmacro MUI_LANGUAGE "Esperanto"
  !insertmacro MUI_LANGUAGE "Asturian"

  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------

Function .onInit
  UserInfo::GetAccountType
  pop $0
  ${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox mb_iconstop "Administrator rights required for installation!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}

  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

;--------------------------------
;Installer section

Section

  ;Remove previous installs
  !include removeprevious.nsh

  SetOutPath "$INSTDIR"

  ;Add all files from folder iQPuzzle into installer (skip .md files)
  File /r /x *.md ${APPNAME}\*.*

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

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

;--------------------------------
;Uninstaller section

Section "Uninstall"

  ;Remove files
  !include removeprevious.nsh
  RMDir "$INSTDIR"

  SetShellVarContext all
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  SetShellVarContext current

  DeleteRegKey HKLM "Software\${APPNAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

SectionEnd
