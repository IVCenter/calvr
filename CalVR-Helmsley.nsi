!include "EnvVarUpdate.nsh"
!include "winmessages.nsh"
!include LogicLib.nsh

; The name of the installer
Name "CalVR-Helmsley"

; The file to write
OutFile "C:\Users\g3aguirre\Documents\CAL\CalVR-Helmsley-installer.exe"

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

Function ReplaceConfig
	; save R1
	Push $R1
	Exch
	; A sequence of replacements.
        ; the string to replace in is at the top of the stack
	Push "{INSTDIR}" ; string to find
	Push "$INSTDIR" ; string to replace it with
	Call StrRep ; see elsewhere in NSIS Wiki
        ; the string to replace in is at the top of the stack again
	Exch 
	Pop $R1
FunctionEnd

Function ReplaceInFile
	Exch $R0 ;file name to search in
	Exch 
	Exch $R4 ;callback function handle
	Push $R1 ;file handle
	Push $R2 ;temp file name
	Push $R3 ;temp file handle
	Push $R5 ;line read
 
	GetTempFileName $R2
  FileOpen $R1 $R0 r ;file to search in
  FileOpen $R3 $R2 w ;temp file
 
  loop_read:
    ClearErrors
    FileRead $R1 $R5 ;read line
    Push $R5 ; put line on stack
    Call $R4
    Pop $R5 ; read line from stack
    IfErrors exit
    FileWrite $R3 $R5 ;write modified line
    Goto loop_read
  exit:
      FileClose $R1
      FileClose $R3
  
      SetDetailsPrint none
      Delete $R0
      Rename $R2 $R0
      Delete $R2
      SetDetailsPrint both
 
	; pop in reverse order
	Pop $R5
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R4
	Pop $R0
FunctionEnd

;--------------------------------

; The stuff to install
Section "CalVR-Helmsley (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\bin
  
  ; Put file there
  File "bin\CalVR.exe"
  File "bin\CalVRAll.dll"
  ;File "bin\freetype.dll"
  File "bin\libcurl.dll"
  File "bin\libpng16.dll"
  File "bin\mxml1.dll"
  File "bin\openvr_api.dll"
  File "bin\osg.dll"
  File "bin\osgAnimation.dll"
  File "bin\osgDB.dll"
  File "bin\osgFX.dll"
  File "bin\osgGA.dll"
  File "bin\osgManipulator.dll"
  File "bin\osgParticle.dll"
  File "bin\osgPresentation.dll"
  File "bin\osgShadow.dll"
  File "bin\osgSim.dll"
  File "bin\osgTerrain.dll"
  File "bin\osgText.dll"
  File "bin\osgUI.dll"
  File "bin\osgUtil.dll"
  File "bin\osgViewer.dll"
  File "bin\osgVolume.dll"
  File "bin\osgWidget.dll"
  File "bin\OpenThreads.dll"
  File "bin\tiff.dll"
  File "bin\tiffxx.dll"
  File "bin\zlib.dll"
  File "bin\glew32.dll"
  File "bin\dcmdata.dll"
  File "bin\dcmimgle.dll"
  File "bin\ofstd.dll"
  File "bin\oflog.dll"



  SetOutpath $INSTDIR\bin\osgPlugins-3.6.5

  File "bin\osgPlugins-3.6.5\osgdb_3dc.dll"
  File "bin\osgPlugins-3.6.5\osgdb_3ds.dll"
  File "bin\osgPlugins-3.6.5\osgdb_ac.dll"
  File "bin\osgPlugins-3.6.5\osgdb_bmp.dll"
  File "bin\osgPlugins-3.6.5\osgdb_bsp.dll"
  File "bin\osgPlugins-3.6.5\osgdb_bvh.dll"
  File "bin\osgPlugins-3.6.5\osgdb_cfg.dll"
  File "bin\osgPlugins-3.6.5\osgdb_curl.dll"
  File "bin\osgPlugins-3.6.5\osgdb_dds.dll"
  File "bin\osgPlugins-3.6.5\osgdb_dot.dll"
  File "bin\osgPlugins-3.6.5\osgdb_dxf.dll"
  File "bin\osgPlugins-3.6.5\osgdb_fbx.dll"
  File "bin\osgPlugins-3.6.5\osgdb_freetype.dll"
  File "bin\osgPlugins-3.6.5\osgdb_gdal.dll"
  File "bin\osgPlugins-3.6.5\osgdb_gif.dll"
  File "bin\osgPlugins-3.6.5\osgdb_gles.dll"
  File "bin\osgPlugins-3.6.5\osgdb_glsl.dll"
  File "bin\osgPlugins-3.6.5\osgdb_gz.dll"
  File "bin\osgPlugins-3.6.5\osgdb_hdr.dll"
  File "bin\osgPlugins-3.6.5\osgdb_ive.dll"
  File "bin\osgPlugins-3.6.5\osgdb_jpeg.dll"
  File "bin\osgPlugins-3.6.5\osgdb_ktx.dll"
  File "bin\osgPlugins-3.6.5\osgdb_logo.dll"
  File "bin\osgPlugins-3.6.5\osgdb_lua.dll"
  File "bin\osgPlugins-3.6.5\osgdb_lwo.dll"
  File "bin\osgPlugins-3.6.5\osgdb_lws.dll"
  File "bin\osgPlugins-3.6.5\osgdb_md2.dll"
  File "bin\osgPlugins-3.6.5\osgdb_mdl.dll"
  File "bin\osgPlugins-3.6.5\osgdb_normals.dll"
  File "bin\osgPlugins-3.6.5\osgdb_obj.dll"
  File "bin\osgPlugins-3.6.5\osgdb_ogr.dll"
  File "bin\osgPlugins-3.6.5\osgdb_openflight.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osc.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osg.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osga.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osgjs.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osgshadow.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osgterrain.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osgtgz.dll"
  File "bin\osgPlugins-3.6.5\osgdb_osgviewer.dll"
  File "bin\osgPlugins-3.6.5\osgdb_p3d.dll"
  File "bin\osgPlugins-3.6.5\osgdb_pic.dll"
  File "bin\osgPlugins-3.6.5\osgdb_ply.dll"
  File "bin\osgPlugins-3.6.5\osgdb_png.dll"
  File "bin\osgPlugins-3.6.5\osgdb_pnm.dll"
  File "bin\osgPlugins-3.6.5\osgdb_pov.dll"
  File "bin\osgPlugins-3.6.5\osgdb_pvr.dll"
  File "bin\osgPlugins-3.6.5\osgdb_revisions.dll"
  File "bin\osgPlugins-3.6.5\osgdb_rgb.dll"
  File "bin\osgPlugins-3.6.5\osgdb_rot.dll"
  File "bin\osgPlugins-3.6.5\osgdb_scale.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osg.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osganimation.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgfx.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgga.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgmanipulator.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgparticle.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgshadow.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgsim.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgterrain.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgtext.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgui.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgutil.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgviewer.dll"
  File "bin\osgPlugins-3.6.5\osgdb_serializers_osgvolume.dll"
  File "bin\osgPlugins-3.6.5\osgdb_shp.dll"
  File "bin\osgPlugins-3.6.5\osgdb_stl.dll"
  File "bin\osgPlugins-3.6.5\osgdb_tf.dll"
  File "bin\osgPlugins-3.6.5\osgdb_tga.dll"
  File "bin\osgPlugins-3.6.5\osgdb_tgz.dll"
  File "bin\osgPlugins-3.6.5\osgdb_tiff.dll"
  File "bin\osgPlugins-3.6.5\osgdb_trans.dll"
  File "bin\osgPlugins-3.6.5\osgdb_trk.dll"
  File "bin\osgPlugins-3.6.5\osgdb_txf.dll"
  File "bin\osgPlugins-3.6.5\osgdb_txp.dll"
  File "bin\osgPlugins-3.6.5\osgdb_vtf.dll"
  File "bin\osgPlugins-3.6.5\osgdb_x.dll"
  File "bin\osgPlugins-3.6.5\osgdb_zip.dll"


  SetOutPath $INSTDIR\bin\calvr-plugins

  File "bin\calvr-plugins\HelmsleyVolume.dll"

  SetOutPath $INSTDIR\config

  File "config\config.xml"
  GetFunctionAddress $R0 ReplaceConfig
  Push $R0
  Push "$INSTDIR\config\config.xml"
  Call ReplaceInFile

  SetOutPath $INSTDIR\HelmsleyVolume\shaders
  File /r "C:\Users\g3aguirre\Documents\CAL\calvr_plugins\calit2\HelmsleyVolume\shaders\"
  SetOutPath $INSTDIR\HelmsleyVolume\models
  File /r "C:\Users\g3aguirre\Documents\CAL\calvr_plugins\calit2\HelmsleyVolume\models\"
  SetOutPath $INSTDIR\HelmsleyVolume\icons
  File /r "C:\Users\g3aguirre\Documents\CAL\calvr_plugins\calit2\HelmsleyVolume\icons\"
  SetOutPath $INSTDIR\HelmsleyVolume\icons\temp
  File /r "C:\Users\g3aguirre\Documents\CAL\calvr_plugins\calit2\HelmsleyVolume\icons\temp\"

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
