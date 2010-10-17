!define R_DOWNLOAD_URL "http://cran.r-project.org/bin/windows/base/"
!define KDE_DOWNLOAD_URL "http://techbase.kde.org/Projects/KDE_on_Windows/Installation"

!include nsDialogs.nsh
!include LogicLib.nsh
!include TextFunc.nsh
!include rkward_version.nsh

Name "RKWard"
outFile "install_rkward_${RKWARD_VERSION}.exe"

# main variables
Var INST_RHOME
Var INST_KDEPREFIX

# page variables
Var Dialog
Var FileSelectLineEdit
Var Button
Var FileSelectCurrent
Var FileBrowseInstructionText
Var FileBrowseValidateFunc
Var FileBrowseStatusLabel
Var MyTemp
Var RHomeOk
Var KDEPrefixOk
Var DownloadLinkText
Var DownloadLinkDest
Var RDll_location

# pages
Page custom WelcomeCreate
PageEx license
	LicenseData "COPYING"
PageExEnd
Page custom RHomeCreate RHomeLeave
Page custom KDEHomeCreate KDEHomeLeave
Page instfiles

!macro MakeExternalLink Py Plabel Plink
	Push $0 ; save $0

	${NSD_CreateLink} 0 ${Py} 100% 16u "${Plabel}"
	Pop $0
	nsDialogs::SetUserData $0 ${Plink}
	GetFunctionAddress $MyTemp OpenExternalLink
	nsDialogs::onClick $0 $MyTemp

	Pop $0 ; restore old value
!macroend

Function FileBrowseButtonClick
	Pop $0
	nsDialogs::SelectFolderDialog /NOUNLOAD $FileBrowseInstructionText $FileSelectCurrent
	Pop $MyTemp
	${If} $MyTemp != error
		${NSD_SetText} $FileSelectLineEdit $MyTemp
	${EndIf}
FunctionEnd

Function FileSelectLineEditChange
	Pop $0
	${NSD_GetText} $FileSelectLineEdit $FileSelectCurrent
	Call $FileBrowseValidateFunc
FunctionEnd

Function OpenExternalLink
	Pop $0
	nsDialogs::getUserData $0
	Pop $0
	ExecShell "open" $0
FunctionEnd

Function WelcomeCreate
	nsDialogs::Create 1018
	Pop $Dialog

	${If} $Dialog == error
		Abort
	${EndIf}

	${NSD_CreateLabel} 0 0 100% 20u "Welcome to the RKWard on Windows installer."
	${NSD_CreateLabel} 0 21u 100% 32u "RKWard on Windows is still young, and installation is a bit cumbersome, but we'll talk you through the needed steps.$\r$\nNote that you need to install R and KDE _before_ you can install RKWard.$\r$\nPlease DO read these instructions, first:"

	!insertmacro MakeExternalLink 62u "READ ME: Installing RKWard on Windows" "http://p.sf.net/rkward/windows"

	${NSD_CreateLabel} 0 86u 100% 16u "These links to R and KDE, may also be helpful:"
	!insertmacro MakeExternalLink 102u "Download R installer" ${R_DOWNLOAD_URL}
	!insertmacro MakeExternalLink 118u "KDE installation instructions" ${KDE_DOWNLOAD_URL}

	nsDialogs::Show
FunctionEnd

# Check wether $FileSelectCurrent qualifies as R home directory
Function ValidateRHome
	Var /Global RHomeOk_count
	StrCpy $RHomeOk_count 0
	StrCpy $RHomeOk "no"

	IfFileExists $FileSelectCurrent\bin\R.exe Rexe_found
		StrCpy $0 "$FileSelectCurrent\bin\R.exe does not exist"
	Goto NextCheck
	Rexe_Found:
		StrCpy $0 "$FileSelectCurrent\bin\R.exe exists"
		IntOp $RHomeOk_count $RHomeOk_count + 1
	NextCheck:
	StrCpy $RDll_location $FileSelectCurrent\bin\R.dll
	IfFileExists $RDll_location Rdll_found
		StrCpy $RDll_location $FileSelectCurrent\bin\i386\R.dll
	IfFileExists $RDll_location Rdll_found
		StrCpy $1 "$FileSelectCurrent\bin[\i386]\R.dll does not exist"
	Goto done
	Rdll_Found:
		GetDllVersion "$RDll_location" $R0 $R1
		IntOp $R2 $R0 / 0x00010000
		IntOp $R3 $R0 & 0x0000FFFF
		StrCpy $1 "Version $R2.$R3"

		# 0x00020005a == Version 2.9(.)0
		${If} $R0 >= 0x0002005a
			StrCpy $2 "should be ok"
			IntOp $RHomeOk_count $RHomeOk_count + 1
		${Else}
			StrCpy $2 "is too old!"
		${EndIf}
		StrCpy $1 "$RDll_location exists ($1 $2)"
	done:
	${If} $RHomeOk_count >= 2
		StrCpy $2 "OK!"
		StrCpy $RHomeOk "yes"
	${Else}
		StrCpy $2 "FAIL!"
	${EndIf}
	${NSD_SetText} $FileBrowseStatusLabel "$0$\r$\n$1$\r$\n$\r$\n$2"

	StrCpy $INST_RHOME $FileSelectCurrent
FunctionEnd

Function RHomeCreate
	${If} $INST_RHOME == ""
		StrCpy $INST_RHOME "$PROGRAMFILES\R"
	${EndIf}

	StrCpy $FileSelectCurrent $INST_RHOME
	StrCpy $FileBrowseInstructionText "Select the directory of your _existing_ R installation. R has to be installed, already, this installer will not do this for you (but see the link at the bottom of this page."
	GetFunctionAddress $FileBrowseValidateFunc ValidateRHome
	StrCpy $DownloadLinkText "Download R installer"
	StrCpy $DownloadLinkDest ${R_DOWNLOAD_URL}

	Call CreateDirSelectionPage
FunctionEnd

Function RHomeLeave
	${If} $RHomeOk != "yes"
		Messagebox MB_OK|MB_ICONINFORMATION \
			"$FileSelectCurrent does not qualify as the directory of the R installation to use.$\r$\nEither R is not installed, there at all, or the installed version is to old."
		Abort ;
	${EndIf}
FunctionEnd

Function KDEHomeCreate
	${If} $INST_KDEPREFIX == ""
		StrCpy $INST_KDEPREFIX "C:\KDE2"
		ExecWait 'cmd.exe /C kde4-config --prefix > "$PLUGINSDIR\kdeprefix.txt"'
		ClearErrors
		FileOpen $0 "$PLUGINSDIR\kdeprefix.txt" r
		IfErrors notinpath
		FileRead $0 $INST_KDEPREFIX
		${TrimNewLines} $INST_KDEPREFIX $INST_KDEPREFIX
		FileClose $0
		notinpath:
	${EndIf}

	StrCpy $FileSelectCurrent $INST_KDEPREFIX
	StrCpy $FileBrowseInstructionText "Select the base directory of your _existing_ KDE installation. KDE has to be installed, already, this installer will not do this for you (but see the link at the bottom of this page."
	GetFunctionAddress $FileBrowseValidateFunc ValidateKDEPrefix
	StrCpy $DownloadLinkText "KDE installation instructions"
	StrCpy $DownloadLinkDest ${KDE_DOWNLOAD_URL}

	Call CreateDirSelectionPage
FunctionEnd

Function KDEHomeLeave
	${If} $KDEPrefixOk != "yes"
		Messagebox MB_OK|MB_ICONINFORMATION \
			"$FileSelectCurrent does not seem to contain a KDE installation."
		Abort ;
	${EndIf}
FunctionEnd

Function ValidateKDEPrefix
	Var /Global KDEPrefixOk_count
	StrCpy $KDEPrefixOk_count 0
	StrCpy $KDEPrefixOk "no"

	IfFileExists $FileSelectCurrent\bin bindir_found
		StrCpy $0 "Directory $FileSelectCurrent\bin does not exist"
	Goto NextCheck
	bindir_found:
		StrCpy $0 "Directory $FileSelectCurrent\bin exists"
		IntOp $KDEPrefixOk_count $KDEPrefixOk_count + 1
	NextCheck:
	IfFileExists $FileSelectCurrent\share sharedir_found
		StrCpy $1 "Directory $FileSelectCurrent\share does not exist"
	Goto done
	sharedir_Found:
		StrCpy $1 "Directory $FileSelectCurrent\share exists"
		IntOp $KDEPrefixOk_count $KDEPrefixOk_count + 1
	done:
	${If} $KDEPrefixOk_count >= 2
		StrCpy $2 "This looks ok."
		StrCpy $KDEPrefixOk "yes"
	${Else}
		StrCpy $2 "This looks wrong."
	${EndIf}
	${NSD_SetText} $FileBrowseStatusLabel "$0$\r$\n$1$\r$\n$\r$\n$2"

	StrCpy $INST_KDEPREFIX $FileSelectCurrent
FunctionEnd

Function CreateDirSelectionPage
	nsDialogs::Create 1018
	Pop $Dialog

	${If} $Dialog == error
		Abort
	${EndIf}

	${NSD_CreateLabel} 0 0 100% 24u $FileBrowseInstructionText

	${NSD_CreateText} 0 35u 70% 16u $FileSelectCurrent
	Pop $FileSelectLineEdit
	GetFunctionAddress $MyTemp FileSelectLineEditChange
	nsDialogs::onChange $FileSelectLineEdit $MyTemp

	${NSD_CreateBrowseButton} 75% 35u 25% 16u "Browse"
	Pop $Button
	GetFunctionAddress $MyTemp FileBrowseButtonClick
	nsDialogs::onClick $Button $MyTemp

	${NSD_CreateLabel} 0 60u 100% 50u ""
	Pop $FileBrowseStatusLabel
	Call $FileBrowseValidateFunc

	!insertmacro MakeExternalLink -16u $DownloadLinkText $DownloadLinkDest

	nsDialogs::Show
FunctionEnd

## installation

Function setRBinaryPath
	StrCpy $R9 "SET R_BINARY=$INST_RHOME\bin\R.exe$\r$\n"
	Push $0
FunctionEnd

# set desktop as install directory
installDir $DESKTOP

# default section
section
	setOutPath $INST_KDEPREFIX
	File /r install\_KDEPREFIX_\*.*
	${LineFind} "$INST_KDEPREFIX\bin\rkward.bat" "" 10 setRBinaryPath

	setOutPath $INST_RHOME
	File /r install\_RHOME_\*.*

	setOutPath $INST_KDEPREFIX\bin
	File "rkward.ico"
	CreateShortCut "$DESKTOP\RKWard.lnk" "$INST_KDEPREFIX\bin\rkward.bat" "" "$INST_KDEPREFIX\bin\rkward.ico"

#	writeUninstaller $INST_KDEPREFIX\uninstaller.exe
sectionEnd
 
#section "Uninstall"
#	# Always delete uninstaller first
#	delete $INSTDIR\uninstaller.exe
#sectionEnd
