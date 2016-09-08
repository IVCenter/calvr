!include "EnvVarUpdate.nsh"
!include "winmessages.nsh"
!include LogicLib.nsh

; The name of the installer
Name "CalVR"

; The file to write
OutFile "CalVR-installer.exe"

; The default installation directory
InstallDir $PROGRAMFILES\CalVR

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\CalVR" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "CalVR (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\bin
  
  ; Put file there
  File "bin\CalVR.exe"
  File "bin\CalVRAll.dll"

  SetOutPath $INSTDIR\bin\calvr-plugins

  File "bin\calvr-plugins\ClipPlane.dll"
  File "bin\calvr-plugins\CollaborativePlugin.dll"
  File "bin\calvr-plugins\MenuBasics.dll"
  File "bin\calvr-plugins\ModelLoader.dll"

  SetOutPath $INSTDIR\config

  File "config\config.xml"
  File "config\oculus.xml"

  SetOutPath $INSTDIR

  File /r "resources"
  File /r "shaders"
  File /r "icons"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\CalVR "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CalVR" "DisplayName" "CalVR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CalVR" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CalVR" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CalVR" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\bin"
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\3rdParty\bin"


  ; HKLM (all users) vs HKCU (current user) defines
  !define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
  !define env_hkcu 'HKCU "Environment"'
  ; set variable
  ReadEnvStr $R0 "OSG_NOTIFY_LEVEL"
  ${If} $R0 == ""
    WriteRegExpandStr ${env_hklm} "OSG_NOTIFY_LEVEL" "always"
  ${EndIf}
  
  ReadEnvStr $R0 "CALVR_CONFIG_DIR"
  ${If} $R0 == ""
    WriteRegExpandStr ${env_hklm} "CALVR_CONFIG_DIR" "$INSTDIR\config"
  ${EndIf}

  ReadEnvStr $R0 "CALVR_CONFIG_FILE"
  ${If} $R0 == ""
    WriteRegExpandStr ${env_hklm} "CALVR_CONFIG_File" "config.xml"
  ${EndIf}

  ReadEnvStr $R0 "CALVR_HOME"
  ${If} $R0 == ""
    WriteRegExpandStr ${env_hklm} "CALVR_HOME" "$INSTDIR"
  ${EndIf}

  ReadEnvStr $R0 "CALVR_HOST_NAME"
  ${If} $R0 == ""
    WriteRegExpandStr ${env_hklm} "CALVR_HOST_NAME" "winInst"
  ${EndIf}

  ; make sure windows knows about the change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

SectionEnd

Section "3rd Party Dependencies (required)"

  SetOutPath $INSTDIR\3rdParty\bin

  File "..\deps\boost\lib\*.dll"
  File "..\deps\Coin\bin\*"
  File "..\deps\collada-dom\bin\*"
  File "..\deps\CURL\bin\*"
  File "..\deps\glew\bin\*"
  File "..\deps\hidapi\bin\*"
  File "..\deps\libxml2\bin\*"
  File "..\deps\mxml\bin\*"
  File /r "..\deps\OpenSceneGraph\bin\*"
  File "..\deps\openvr\bin\win64\*.dll"
  File "..\deps\png\bin\*"
  File "..\deps\SDL\bin\*"
  File "..\deps\simage\bin\*"
  File "..\deps\tiff\bin\*"
  File "..\deps\vrpn\bin\*"
  File "..\deps\zlib\bin\*"

  SetOutPath $INSTDIR\3rdParty\include

  File /r "..\deps\boost\include\*"
  File /r "..\deps\Coin\include\*"
  File /r "..\deps\collada-dom\include\*"
  File /r "..\deps\CURL\include\*"
  File /r "..\deps\freetype\include\*"
  File /r "..\deps\gdal\include\*"
  File /r "..\deps\gif\include\*"
  File /r "..\deps\glew\include\*"
  File /r "..\deps\hidapi\include\*"
  File /r "..\deps\jpeg\include\*"
  File /r "..\deps\libxml2\include\*"
  File /r "..\deps\mxml\include\*"
  File /r "..\deps\OpenSceneGraph\include\*"
  File /r "..\deps\openvr\headers\*"
  File /r "..\deps\png\include\*"
  File /r "..\deps\SDL\include\*"
  File /r "..\deps\simage\include\*"
  File /r "..\deps\tiff\include\*"
  File /r "..\deps\vrpn\include\*"
  File /r "..\deps\zlib\include\*"

  SetOutPath $INSTDIR\3rdParty\lib

  File "..\deps\boost\lib\*.lib"
  File "..\deps\Coin\lib\*"
  File "..\deps\collada-dom\lib\*.lib"
  File "..\deps\CURL\lib\*.lib"
  File "..\deps\freetype\lib\*"
  File /r "..\deps\gdal\lib\*"
  File "..\deps\gif\lib\*"
  File "..\deps\glew\lib\*"
  File "..\deps\hidapi\lib\*"
  File "..\deps\jpeg\lib\*"
  File "..\deps\libxml2\lib\*"
  File "..\deps\mxml\lib\*"
  File "..\deps\OpenSceneGraph\lib\*.lib"
  File "..\deps\openvr\lib\win64\*.lib"
  File /r "..\deps\png\lib\*"
  File "..\deps\SDL\lib\*"
  File "..\deps\simage\lib\*"
  File "..\deps\tiff\lib\*.lib"
  File "..\deps\vrpn\lib\*"
  File "..\deps\zlib\lib\*"

SectionEnd

Section "Plugin Development Headers"

  SetOutPath $INSTDIR

  File /r "include"
  
  SetOutPath $INSTDIR\lib

  File "lib\CalVRAll.lib"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\CalVR"
  CreateShortCut "$SMPROGRAMS\CalVR\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\CalVR\CalVR.lnk" "$INSTDIR\bin\CalVR.exe" "" "$INSTDIR\bin\CalVR.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CalVR"
  DeleteRegKey HKLM SOFTWARE\CalVR

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\CalVR\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\CalVR"
  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR\include"
  RMDir /r "$INSTDIR\resources"
  RMDir /r "$INSTDIR\shaders"
  RMDir /r "$INSTDIR\icons"
  RMDir /r "$INSTDIR\3rdParty"
  RMDir /r "$INSTDIR"

  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\3rdParty\bin"
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"

  DeleteRegValue ${env_hklm} "OSG_NOTIFY_LEVEL"
  DeleteRegValue ${env_hklm} "CALVR_CONFIG_DIR"
  DeleteRegValue ${env_hklm} "CALVR_CONFIG_FILE"
  DeleteRegValue ${env_hklm} "CALVR_HOME"
  DeleteRegValue ${env_hklm} "CALVR_HOST_NAME"
  ; make sure windows knows about the change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

SectionEnd
