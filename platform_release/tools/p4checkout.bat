@ECHO OFF

set OLDP4CLIENT=%P4CLIENT%
set OLDP4USER=%P4USER%
set OLDP4PORT=%P4PORT%

if not "_%1%_" == "_help_" (
    goto BEGIN
)

echo.
echo     %0% -- The CVS-like anonymous snapshot download script for stlab.adobe.com
echo.
echo     This script is intended for those who would like a read-only snapshot of the
echo     source code depots at stlab.adobe.com:10666. It creates a temporary client
echo     specification on the Perforce server to mimic the beahvior of CVS 'checkout'
echo     command, and then deletes the client spec once the sync has completed. The
echo     files will be placed in a folder at the current working directory of the
echo     script."
echo.
echo     If you already have a P4PORT specified in your environment, the script will use
echo     that value instead of the default (stlab.adobe.com:10666)
echo.
echo     The script takes one (optional) parameter, which is the path of the depot to
echo     which you would like to sync. If you do not specify a depot, the official
echo     distribution depot (release) will be used. For instance, running:
echo.
echo         %0% //source_release/...
echo.
echo     ... will sync to the source_release depot, where you are likely to find patches
echo     and updates to the sources between official releases.
echo.
echo     Please note: if you are an Adobe Source Libraries developer, this is not
echo     the tool for you, as the client specification is not retained, and you will
echo     not be able to check in any changes to any depot.
echo.
goto END

:BEGIN

echo ***
echo *** P4 Checkout Script. Type '%0% help' for help.
echo *** Copyright 2005-2007 Adobe Systems Incorporated
echo *** Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
echo *** or a copy at http://stlab.adobe.com/licenses.html)
echo ***

set MSVC71PATH1=%PROGRAMFILES%\Microsoft Visual Studio .NET 2003\Common7\Tools
set MSVC71PATH2=%PROGRAMFILES%\Microsoft Visual Studio .NET 2003\Common7\Tools\Bin
set MSVC80PATH1=%PROGRAMFILES%\Microsoft Visual Studio 8\Common7\Tools
set MSVC80PATH2=%PROGRAMFILES%\Microsoft Visual Studio 8\Common7\Tools\Bin



if exist "%MSVC80PATH2%" (
    CALL "%MSVC80PATH2%\uuidgen.exe" > %TEMP%\adobe_uuid.txt
    goto GRABS
)

if exist "%MSVC80PATH1%" (
    CALL "%MSVC80PATH1%\uuidgen.exe" > %TEMP%\adobe_uuid.txt
    goto GRABS
)

if exist "%MSVC71PATH1%" (
    CALL "%MSVC71PATH1%\uuidgen.exe" > %TEMP%\adobe_uuid.txt
    goto GRABS
)

if exist "%MSVC71PATH2%" (
    CALL "%MSVC71PATH2%\uuidgen.exe" > %TEMP%\adobe_uuid.txt
    goto GRABS
)


echo ***
echo *** A UUID-generating utility could not be found to make the clientspec name.
echo *** Please make sure you have MSVC 2003 or MSVC 8 installed, and try again.
echo ***

goto END

:GRABS

popd

set /P UUID="" < %TEMP%\adobe_uuid.txt

rem echo UUID is "%UUID%"

set SOURCE_DEPOT=//source_release/...
set PLATFORM_DEPOT=//platform_release/...

if NOT "_%1%_" == "__" (
    set SOURCE_DEPOT=%1%
    set PLATFORM_DEPOT=""
)

if "_%P4PORT%_" == "__" (
    set P4PORT=stlab.adobe.com:10666
)

set P4USER=guest
set P4CLIENT=%UUID%
set THEPASS=AdobeGuest

rem *** Here we create the temporary client spec, do the sync, and delete the spec

p4 -P %THEPASS% client -o %P4CLIENT% > %TEMP%\adobe_client.txt

p4 -P %THEPASS% client -i < %TEMP%\adobe_client.txt > NUL

p4 -P %THEPASS% sync -f %SOURCE_DEPOT%

p4 -P %THEPASS% sync -f %PLATFORM_DEPOT%

p4 -P %THEPASS% client -d %P4CLIENT% > NUL

:END

rem *** now we clean up after ourselves.

pushd %TEMP%

if exist adobe_uuid.txt del adobe_uuid.txt
if exist adobe_client.txt del adobe_client.txt

popd

set P4CLIENT=%OLDP4CLIENT%
set P4USER=%OLDP4USER%
set P4PORT=%OLDP4PORT%
set OLDP4CLIENT=
set OLDP4USER=
set OLDP4PORT=

exit /B 0
