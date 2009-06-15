SET KDEPREFIXDRIVE=c:
SET KDEPREFIX=KDE
SET RHOMEDRIVE=c:
SET RHOME=Programme/R/R-2.9.0

SET SH_PATH=C:\Rtools\bin
SET MAKENSIS=c:/programme/nsis/makensis.exe

REM --------------------------------------------
REM everything below should not need customizing
REM --------------------------------------------

SET ADD_CMAKE_FLAGS=" -DCMAKE_BUILD_TYPE=Release "

SET RELEASE_DIR=%~dp0
SET SOURCE_DIR=%RELEASE_DIR%\..
SET INSTALL_DIR=%RELEASE_DIR%/install

cd %RELEASE_DIR%

rmdir /Q /S %RELEASE_DIR%\install
mkdir install
mkdir build
cd build
cmake %SOURCE_DIR% -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=%KDEPREFIXDRIVE%/%KDEPREFIX% -DR_EXECUTABLE=%RHOMEDRIVE%/%RHOME%/bin/R.exe -DR_LIBDIR=%RHOMEDRIVE%/%RHOME%/library %ADD_CMAKE_FLAGS%
REM sh.exe must not be in path during cmake call, but must be in path for R package install...
SET PATH=%PATH%;%SH_PATH%
mingw32-make install DESTDIR=%INSTALL_DIR%

move %INSTALL_DIR%/%KDEPREFIX% %INSTALL_DIR%/_KDEPREFIX_
move %INSTALL_DIR%/%RHOME% %INSTALL_DIR%/_RHOME_

REM Prepare Version info for use in NSIS
echo !define RKWARD_VERSION \ > %RELEASE_DIR%/rkward_version.nsh
more %SOURCE_DIR%\rkward\resource.ver >> %RELEASE_DIR%/rkward_version.nsh
copy /Y %SOURCE_DIR%\COPYING %RELEASE_DIR%

cd %RELEASE_DIR%
%MAKENSIS% installer.nsi
