; Installation script for KiCad generated by Alastair Hoyle
;
; This installation script requires NSIS (Nullsoft Scriptable Install System) version 2.18 http://nsis.sourceforge.net/Main_Page
; This script was created with HM NIS Edit version 2.0.3 http://hmne.sourceforge.net/
;
; This script is provided as is with no warranties and has only been tested on Windows XP.
; Copyright (C) 2006 Alastair Hoyle <ahoyle@hoylesolutions.co.uk>
; This program is free software; you can redistribute it and/or modify it under the terms of the
; GNU General Public License as published by the Free Software Foundation. This program is distributed
; in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
;
; This script should be in a subdirectory of the full build directory (Kicad/NSIS by default)
; When the build is updated the product and installer versions should be updated before recompiling the installation file
;
; This script expects the install.ico, uninstall.ico, language and license files to be in the same directory as this script

; General Product Description Definitions
!define PRODUCT_NAME "KiCad"
!define PRODUCT_VERSION "2011.10.15"
!define PRODUCT_WEB_SITE "http://iut-tice.ujf-grenoble.fr/kicad/"
!define SOURCEFORGE_WEB_SITE "http://kicad.sourceforge.net/"
!define COMPANY_NAME ""
!define TRADE_MARKS ""
!define COPYRIGHT "Kicad Team (Jean-Pierre Charras et all)"
!define COMMENTS ""
!define HELP_WEB_SITE "http://groups.yahoo.com/group/kicad-users/"
!define DEVEL_WEB_SITE "https://launchpad.net/~kicad-developers/"
!define WINGS3D_WEB_SITE "http://www.wings3d.com"

!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define UNINST_ROOT "HKLM"

;Comment out the following SetCompressor command while testing this script
SetCompressor /final /solid lzma
CRCCheck force
XPStyle on
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME}.exe"
InstallDir "$PROGRAMFILES\KiCad"
ShowInstDetails hide
ShowUnInstDetails hide

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "install.ico"
!define MUI_UNICON "uninstall.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${UNINST_ROOT}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Installer pages
!define MUI_CUSTOMFUNCTION_GUIINIT myGuiInit
!define MUI_CUSTOMFUNCTION_UNGUIINIT un.myGuiInit
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE $(MUILicense)
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_SHOWREADME ${WINGS3D_WEB_SITE}
!define MUI_FINISHPAGE_SHOWREADME_TEXT "text"
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_PAGE_CUSTOMFUNCTION_PRE ModifyFinishPage
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
; - To add another language; add an insert macro line here and inlcude a language file as below
; - This must be after all page macros have been inserted
!insertmacro MUI_LANGUAGE "English" ;first language is the default language
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Japanese"

!include "English.nsh"
!include "French.nsh"
!include "Polish.nsh"
!include "Dutch.nsh"
!include "Russian.nsh"
!include "Japanese.nsh"

; MUI end ------

Function .onInit
  ReserveFile "install.ico"
  ReserveFile "uninstall.ico"
  ReserveFile "${NSISDIR}\Plugins\InstallOptions.dll"
  ReserveFile "${NSISDIR}\Plugins\LangDLL.dll"
  ReserveFile "${NSISDIR}\Plugins\System.dll"
  ReserveFile "${NSISDIR}\Contrib\Modern UI\ioSpecial.ini"
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Function myGuiInit
  Call PreventMultiInstances
  Call CheckAlreadyInstalled
FunctionEnd

Function ModifyFinishPage
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 4" "Text" $(WINGS3D_PROMPT)
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 4" "Bottom" 168                 ;make more space for prompt
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSpecial.ini" "Settings" "CancelShow" "0"            ;hide cancel button - already installed!!
FunctionEnd

Section $(TITLE_SEC01) SEC01
  SectionIn RO
  SetOverwrite try
  SetOutPath "$INSTDIR"
  File /nonfatal "..\AUTHORS.txt"
  File /nonfatal "..\copyright.txt"
  File /nonfatal "..\gpl.txt"
  File /nonfatal "..\licendoc.txt"
  SetOutPath "$INSTDIR\share\template"
  File /nonfatal /r "..\share\template\*"
  SetOutPath "$INSTDIR\bin"
  File /r "..\bin\*"
  SetOutPath "$INSTDIR\share\internat"
  File /r "..\share\internat\*"
SectionEnd

Section $(TITLE_SEC02) SEC02
  SetOverwrite try
  SetOutPath "$INSTDIR\share\library"
  File /nonfatal /r "..\share\library\*"
  SetOutPath "$INSTDIR\share\modules"
  File /nonfatal /r "..\share\modules\*"
SectionEnd

Section $(TITLE_SEC03) SEC03
  SetOverwrite try
  SetOutPath "$INSTDIR\share\demos"
  File /nonfatal /r "..\share\demos\*"
  SetOutPath "$INSTDIR\doc\tutorials"
  File /nonfatal /r "..\doc\tutorials\*"
SectionEnd

Section $(TITLE_SEC04) SEC04
  SetOverwrite try
  SetOutPath "$INSTDIR\doc\help"
  File /nonfatal /r "..\doc\help\*"
SectionEnd

Section -CreateShortcuts
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\HomePage.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  WriteIniStr "$INSTDIR\SourceForge.url" "InternetShortcut" "URL" "${SOURCEFORGE_WEB_SITE}"
  WriteIniStr "$INSTDIR\UserGroup.url" "InternetShortcut" "URL" "${HELP_WEB_SITE}"
  WriteIniStr "$INSTDIR\DevelGroup.url" "InternetShortcut" "URL" "${DEVEL_WEB_SITE}"
  WriteIniStr "$INSTDIR\Wings3D.url" "InternetShortcut" "URL" "${WINGS3D_WEB_SITE}"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\KiCad"
  CreateShortCut "$SMPROGRAMS\KiCad\Home Page.lnk" "$INSTDIR\HomePage.url"
  CreateShortCut "$SMPROGRAMS\KiCad\Kicad SourceForge.lnk" "$INSTDIR\SourceForge.url"
  CreateShortCut "$SMPROGRAMS\KiCad\User Group.lnk" "$INSTDIR\UserGroup.url"
  CreateShortCut "$SMPROGRAMS\KiCad\Devel Group.lnk" "$INSTDIR\DevelGroup.url"
  CreateShortCut "$SMPROGRAMS\KiCad\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
  CreateShortCut "$SMPROGRAMS\KiCad\KiCad.lnk" "$INSTDIR\bin\kicad.exe"
  CreateShortCut "$SMPROGRAMS\KiCad\Wings3D.lnk" "$INSTDIR\Wings3D.url"
  CreateShortCut "$DESKTOP\KiCad.lnk" "$INSTDIR\bin\kicad.exe"
SectionEnd

Section -CreateAddRemoveEntry
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "Publisher" "${COMPANY_NAME}"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstaller.exe"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\kicad.exe"
  WriteRegDWORD ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "NoModify" "1"
  WriteRegDWORD ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "NoRepair" "1"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "Comments" "${COMMENTS}"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "HelpLink" "${HELP_WEB_SITE}"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"

  WriteUninstaller "$INSTDIR\uninstaller.exe"
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} $(DESC_SEC01)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} $(DESC_SEC02)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} $(DESC_SEC03)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} $(DESC_SEC04)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Function un.myGuiInit
  Call un.PreventMultiInstances
  MessageBox MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 $(UNINST_PROMPT) /SD IDYES IDYES +2
  Abort
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK $(UNINST_SUCCESS) /SD IDOK
FunctionEnd

Section Uninstall
  ;delete uninstaller first
  Delete "$INSTDIR\uninstaller.exe"

  ;remove start menu shortcuts and web page links
  SetShellVarContext all
  Delete "$SMPROGRAMS\KiCad\Home Page.lnk"
  Delete "$SMPROGRAMS\KiCad\User Group.lnk"
  Delete "$SMPROGRAMS\KiCad\Uninstall.lnk"
  Delete "$SMPROGRAMS\KiCad\KiCad.lnk"
  Delete "$SMPROGRAMS\KiCad\Wings3D.lnk"
  Delete "$DESKTOP\KiCad.lnk"
  Delete "$INSTDIR\Wings3D.url"
  Delete "$INSTDIR\HomePage.url"
  Delete "$INSTDIR\UserGroup.url"
  RMDir "$SMPROGRAMS\KiCad"

  ;remove all program files now
  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\library"
  RMDir /r "$INSTDIR\modules"
  RMDir /r "$INSTDIR\template"
  RMDir /r "$INSTDIR\internat"
  RMDir /r "$INSTDIR\demos"
  RMDir /r "$INSTDIR\tutorials"
  RMDir /r "$INSTDIR\help"
  RMDir /r "$INSTDIR\share\library"
  RMDir /r "$INSTDIR\share\modules"
  RMDir /r "$INSTDIR\share\template"
  RMDir /r "$INSTDIR\share\internat"
  RMDir /r "$INSTDIR\share\demos"
  RMDir /r "$INSTDIR\doc\tutorials"
  RMDir /r "$INSTDIR\doc\help"
  RMDir /r "$INSTDIR\wings3d"
  ;don't remove $INSTDIR recursively just in case the user has installed it in c:\ or
  ;c:\program files as this would attempt to delete a lot more than just this package
  Delete "$INSTDIR\*.txt"
  RMDir "$INSTDIR"

  ;Note - application registry keys are stored in the users individual registry hive (HKCU\Software\kicad".
  ;It might be possible to remove these keys as well but it would require a lot of testing of permissions
  ;and access to other people's registry entries. So for now we will leave the application registry keys.

  ;remove installation registary keys
  DeleteRegKey ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd

;Setup.exe file version information
  VIProductVersion "${PRODUCT_VERSION}.01"  ;must be in x.x.x.x format - product version plus last digit for installer version number
  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PRODUCT_NAME}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "${COMMENTS}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${COMPANY_NAME}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${TRADE_MARKS}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "� ${COPYRIGHT}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${PRODUCT_NAME} Installer"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PRODUCT_VERSION}"

Function PreventMultiInstances
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION $(INSTALLER_RUNNING) /SD IDOK
  Abort
FunctionEnd

Function un.PreventMultiInstances
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION $(UNINSTALLER_RUNNING) /SD IDOK
  Abort
FunctionEnd

Function CheckAlreadyInstalled
  ReadRegStr $R0 ${UNINST_ROOT} "${PRODUCT_UNINST_KEY}" "DisplayName"
  StrCmp $R0 "" +3
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION $(ALREADY_INSTALLED) /SD IDOK IDOK +2
  Abort
FunctionEnd
