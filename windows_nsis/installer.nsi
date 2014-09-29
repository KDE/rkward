!define R_DOWNLOAD_URL "http://cran.r-project.org/bin/windows/base/"
!define KDE_DOWNLOAD_URL "http://techbase.kde.org/Projects/KDE_on_Windows/Installation"

!include nsDialogs.nsh
!include LogicLib.nsh
!include TextFunc.nsh
!include WordFunc.nsh
!insertmacro "VersionCompare"
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
Var MyTempB
Var RHomeOk
Var KDEPrefixOk
Var DownloadLinkText
Var DownloadLinkDest
Var RDll_location
Var Libktexteditor_dll_location
Var INST_KDE_VERSION

# pages
Page custom WelcomeCreate
PageEx license
	LicenseData "COPYING"
PageExEnd
Page custom RHomeCreate RHomeLeave
Page custom KDEHomeCreate KDEHomeLeave
Page instfiles

# StrRep function. Taken from http://nsis.sf.net/StrRep
!define StrRep "!insertmacro StrRep"
!macro StrRep output string old new
    Push `${string}`
    Push `${old}`
    Push `${new}`
    !ifdef __UNINSTALL__
        Call un.StrRep
    !else
        Call StrRep
    !endif
    Pop ${output}
!macroend
 
!macro Func_StrRep un
    Function ${un}StrRep
        Exch $R2 ;new
        Exch 1
        Exch $R1 ;old
        Exch 2
        Exch $R0 ;string
        Push $R3
        Push $R4
        Push $R5
        Push $R6
        Push $R7
        Push $R8
        Push $R9
 
        StrCpy $R3 0
        StrLen $R4 $R1
        StrLen $R6 $R0
        StrLen $R9 $R2
        loop:
            StrCpy $R5 $R0 $R4 $R3
            StrCmp $R5 $R1 found
            StrCmp $R3 $R6 done
            IntOp $R3 $R3 + 1 ;move offset by 1 to check the next character
            Goto loop
        found:
            StrCpy $R5 $R0 $R3
            IntOp $R8 $R3 + $R4
            StrCpy $R7 $R0 "" $R8
            StrCpy $R0 $R5$R2$R7
            StrLen $R6 $R0
            IntOp $R3 $R3 + $R9 ;move offset by length of the replacement string
            Goto loop
        done:
 
        Pop $R9
        Pop $R8
        Pop $R7
        Pop $R6
        Pop $R5
        Pop $R4
        Pop $R3
        Push $R0
        Push $R1
        Pop $R0
        Pop $R1
        Pop $R0
        Pop $R2
        Exch $R1
    FunctionEnd
!macroend
!insertmacro Func_StrRep ""
!insertmacro Func_StrRep "un."

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

		# 0x000300000 == Version 3.0(.)0 (if think)
		${If} $R0 >= 0x00030000
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
		Messagebox MB_YESNO|MB_ICONSTOP \
			"$FileSelectCurrent does not qualify as the directory of the R installation to use.$\r$\nEither R is not installed, there at all, or the installed version is to old.$\r$\n \
			If you think you now what you are doing, click $\"Yes$\" to continue with the installation, anyway. Otherwise click $\"No$\".$\r$\n$\r$\n \
			Continue with the current settings?" \
			/SD IDNO IDYES next IDNO stop
		stop:
			Abort ;
		next:
	${EndIf}
FunctionEnd

!macro RunSimpleQuery Query Default
	StrCpy $MyTemp "${Default}"

	ClearErrors
	GetTempFileName $R0
	ExecWait 'cmd.exe /C ${Query} > $R0'
	IfErrors skip

	FileOpen $MyTempB $R0 r
	FileSeek $MyTempB 0 SET
	FileRead $MyTempB $MyTemp
	${TrimNewLines} $MyTemp $MyTemp
	FileClose $MyTempB
	ClearErrors

	skip:

	Push $MyTemp
!macroend

Function KDEHomeCreate
	${If} $INST_KDEPREFIX == ""
		!insertmacro RunSimpleQuery "kde4-config.exe --prefix" "C:\KDE"
		Pop $INST_KDEPREFIX
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
		Messagebox MB_YESNO|MB_ICONSTOP \
			"$FileSelectCurrent does not seem to contain a (valid) KDE installation.$\r$\n \
			If you think you now what you are doing, click $\"Yes$\" to continue with the installation, anyway. Otherwise click $\"No$\".$\r$\n$\r$\n \
			Continue with the current settings?" \
			/SD IDNO IDYES next IDNO stop
		stop:
			Abort ;
		next:
	${EndIf}
FunctionEnd

Function ValidateKDEPrefix
	Var /Global KDEPrefixOk_count
	StrCpy $KDEPrefixOk_count 0
	StrCpy $KDEPrefixOk "no"
	StrCpy $1 ""
	StrCpy $2 ""
	StrCpy $Libktexteditor_dll_location $FileSelectCurrent\bin\libktexteditor.dll

	IfFileExists $FileSelectCurrent\bin bindir_found
		StrCpy $0 "Directory $FileSelectCurrent\bin does not exist"
	Goto NextCheck
	bindir_found:
		StrCpy $0 "Directory $FileSelectCurrent\bin exists"
		IntOp $KDEPrefixOk_count $KDEPrefixOk_count + 1

	NextCheck:
	IfFileExists $Libktexteditor_dll_location libktexteditor_dll_found
		StrCpy $1 "$Libktexteditor_dll_location does not exist"
	Goto done
	libktexteditor_dll_found:
		StrCpy $1 "$Libktexteditor_dll_location exists"

		!insertmacro RunSimpleQuery "$\"$FileSelectCurrent\bin\kde4-config.exe$\" --kde-version" "UNKNOWN"
		Pop $INST_KDE_VERSION

		!insertmacro VersionCompareCall $INST_KDE_VERSION "4.10.2" $MyTemp
		${If} $MyTemp < 2
			StrCpy $2 "Version $INST_KDE_VERSION should be ok"
			IntOp $KDEPrefixOk_count $KDEPrefixOk_count + 1
		${Else}
			StrCpy $2 "Version $INST_KDE_VERSION is too old (4.10.2 or later is required)"
			Goto done
		${EndIf}

	done:
	${If} $KDEPrefixOk_count >= 2
		StrCpy $3 "This looks ok."
		StrCpy $KDEPrefixOk "yes"
	${Else}
		StrCpy $3 "This looks wrong."
	${EndIf}
	${NSD_SetText} $FileBrowseStatusLabel "$0$\r$\n$1$\r$\n$2$\r$\n$\r$\n$3"

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

# set desktop as install directory
installDir $DESKTOP

# default section
section
	setOutPath $INST_KDEPREFIX
	File /r install\_KDEPREFIX_\*.*
	FileOpen $4 "$INST_KDEPREFIX\bin\rkward.ini" w
	${StrRep} $5 "R executable=$INST_RHOME\bin\R.exe$\r$\n" "\" "\\"
	FileWrite $4 $5
	FileClose $4

	setOutPath $INST_RHOME
	File /r install\_RHOME_\*.*

	setOutPath $INST_KDEPREFIX\bin
	File "rkward.ico"
#	setOutPath %USERPROFILE%
	CreateShortCut "$DESKTOP\RKWard.lnk" "$INST_KDEPREFIX\bin\rkward.bat" "" "$INST_KDEPREFIX\bin\rkward.ico"

#	writeUninstaller $INST_KDEPREFIX\uninstaller.exe
sectionEnd
 
#section "Uninstall"
#	# Always delete uninstaller first
#	delete $INSTDIR\uninstaller.exe
#sectionEnd
