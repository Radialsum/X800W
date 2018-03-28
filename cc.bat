@echo off
::
:: Simple driver script for compiling single C++ source file.
::

::
:: WARNING: This is just a crude solution to compile a single file.
::    May be something like CMake/make/et al. is what you are looking for.
::
:: WARNING: This script may fail if the arguments has double quotes,
::       parenthesis, ampersand, angle-brackets, spaces, equal-sign etc...
::
:: The compiler environment is set by helper batch scrips.
:: For example, msvcenv.bat or vce.bat sets up the 64-bit environment for
:: VC++ 2015 community edition by executing the following command:
::
::      call "{VS-INSTALL-DIR}\VC\vcvarsall.bat" amd64
::
:: Often the 'vc' prefix is used with VC++ environment setup batch files,
:: and the 'g' prefix is used with MinGW[-w64]/GCC env. setup batch files,
:: and the 'l' prefix is used with LLVM/Clang env. setup batch files.
::
:: Also the compiler environment setup batchfiles are found inside:
::      E:\local\bin
:: Perhaps SUBST or mklink may be used to map E: or E:\local
::

set ARGS=
setlocal ENABLEDELAYEDEXPANSION
setlocal

set CID=%~1
set NAME=%~n0

:: NOP is used as a dummy command; "echo." can be used
set NOP=echo.

if "%CID%" == "?" goto usage
if "%CID%" == "/?" goto usage
if "%CID%" == "-?" goto usage
if "%CID%" == "-h" goto usage

if "%CID%" == "0" goto clean
if "%CID%" == "00" goto distclean

:: expecting DATE be in the format YYYY-MM-DD
set "TM=%TIME: =0%"
set CC_STUB_TS=%DATE:~0,4%%DATE:~5,2%%DATE:~-2%.%TM:~0,2%%TM:~3,2%%TM:~-5%0
set TM=

if [%2]==[] goto usage

set PRI_TC=
set TOOLCHAIN=
call :get_args %*
set OPTS=%ARGS%

if "%CID%" == "1"  set TOOLCHAIN=cpplint&   goto lint_setup
if "%CID%" == "11" set TOOLCHAIN=flintpp&   goto lint_setup
if "%CID%" == "2"  set TOOLCHAIN=cppcheck&  goto lint_setup
if "%CID%" == "3"  set TOOLCHAIN=pmdcpd&    goto lint_setup
if "%CID%" == "31" set TOOLCHAIN=sloppy&    goto lint_setup

set CC_STUB_VC_VER=
set CC_STUB_GC_VER=
set AUX_TC=

if "%CID%" == "5"   set PRI_TC=l50.bat&   set TOOLCHAIN=lcvc& goto cc_env_setup
if "%CID%" == "6"   set PRI_TC=l60.bat&   set TOOLCHAIN=lcvc& goto cc_env_setup
if "%CID%" == "7"   set PRI_TC=g73.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "b"   set PRI_TC=vcb.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "c"   set PRI_TC=vcc.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "e"   set PRI_TC=vce.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "f"   set PRI_TC=vcf.bat&   set TOOLCHAIN=msvc& goto cc_env_setup

if "%CID%" == "3v"  set PRI_TC=l39.bat&   set TOOLCHAIN=lcvc& goto cc_env_setup
if "%CID%" == "4v"  set PRI_TC=l40.bat&   set TOOLCHAIN=lcvc& goto cc_env_setup
if "%CID%" == "3g"  set PRI_TC=l39.bat&   set TOOLCHAIN=lcgc& goto cc_env_setup
if "%CID%" == "4g"  set PRI_TC=l40.bat&   set TOOLCHAIN=lcgc& goto cc_env_setup
if "%CID%" == "49"  set PRI_TC=g49.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "53"  set PRI_TC=g53.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "62"  set PRI_TC=g62.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "63"  set PRI_TC=g63.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "71"  set PRI_TC=g71.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "72"  set PRI_TC=g72.bat&   set TOOLCHAIN=gcc&  goto cc_env_setup
if "%CID%" == "11"  set PRI_TC=vcb.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "12"  set PRI_TC=vcc.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "143" set PRI_TC=vce32.bat& set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "14"  set PRI_TC=vce.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if "%CID%" == "15"  set PRI_TC=vcf.bat&   set TOOLCHAIN=msvc& goto cc_env_setup
if not "%CID%" == "@" goto error_command_id

call :where_is cl.exe
if !ERRORLEVEL! EQU 0 (
  call :where_is clang++.exe
  if !ERRORLEVEL! EQU 0 (
    set TOOLCHAIN=lcvc
  ) else (
    set TOOLCHAIN=msvc
  )
  set AUX_TC=%NOP%
  call :get_msvc_version %NOP%
)

call :where_is g++.exe
if !ERRORLEVEL! EQU 0 (
  call :where_is clang.exe
  if !ERRORLEVEL! EQU 0 (
    set TOOLCHAIN=lcgc
  ) else (
    set TOOLCHAIN=gcc
  )
  set AUX_TC=%NOP%
  call :get_gcc_version %NOP%
)

if not [%TOOLCHAIN%]==[] (
  set PRI_TC=%NOP%
  goto cc_stage2_env_setup
)

goto error_incomplete_env

:lint_setup
set LINTFLAGS=
goto %TOOLCHAIN%

:cpplint
set PRI_TC=cpplint.bat
set LINTFLAGS=
goto run_lint

:cppcheck
set PRI_TC=cppchk.bat
set LINTFLAGS=--quiet --force --enable=all --template=gcc
goto run_lint

:flintpp
set PRI_TC="flint++.exe"
set LINTFLAGS=--level=3
goto run_lint

:sloppy
set PRI_TC=sloppy.exe
set LINTFLAGS=
goto run_lint

:pmdcpd
set PRI_TC=pmdcpd.bat
set LINTFLAGS=
goto run_lint

:run_lint
call :where_is %PRI_TC%
if errorlevel 1 goto error_lint_bat
::echo %PRI_TC% %LINTFLAGS% %ARGS%
cmd /d /c "%PRI_TC% %LINTFLAGS% %ARGS% 2>&1"
goto :eof

:cc_env_setup
call :where_is %PRI_TC%
if errorlevel 1 goto error_env_bat
:: since PRI_TC is fixed, good to reset INCLUDE/LIB/LIBPATH/PATH
set "PATH=C:\Windows;C:\Windows\System32\Wbem;C:\Windows\system32;E:\local\bin"
set INCLUDE=
set LIBPATH=
set LIB=
:cc_stage2_env_setup
call :process_args %*
set CXX=
set CXXFLAGS=
::if "%NAME%"=="ccc" set CXXFLAGS=-fsyntax-only
goto %TOOLCHAIN%
:: TODO - %TOOLCHAIN% is undefined; why?
echo no-file:0:0: error: incorrect compiler environment setup command
goto :eof

:gcc
set CXX=g++
set CXXFLAGS=%CXXFLAGS% -Wall -Wextra -std=c++11
goto compile

:lcgc
if [%AUX_TC%]==[] set AUX_TC=gnucenv.bat
set CXX=clang++
set CXXFLAGS=%CXXFLAGS% --target=x86_64-w64-mingw32 -std=c++11 -Wall -Wextra
goto compile

:lcvc
if [%AUX_TC%]==[] set AUX_TC=msvcenv.bat
set CXX=clang++
set CXXFLAGS=%CXXFLAGS% -Wall -Wextra
goto compile

:msvc
set CXX=cl
if "%NAME%"=="ccc" set CXXFLAGS=-Zs
::
if [%WARN%]==[-Wall] (
  set CXXFLAGS=%CXXFLAGS% -nologo %EH% -DHAS_WALL
) else if not [%WARN%]==[] (
  set CXXFLAGS=%CXXFLAGS% -nologo %EH%
) else (
  set CXXFLAGS=%CXXFLAGS% -nologo -W4 %EH%
)
goto compile

:compile
set CCMD=%CXX% %CXXFLAGS%
set OPTS=%CCMD% %OPTS%
::echo 2OPTS=[%OPTS%]
if [%AUX_TC%]==[] goto no_aux_env
call :where_is %AUX_TC%
if errorlevel 1 goto error_env_bat
if [%AUX_TC:~0,1%]==[g] call :get_gcc_version %AUX_TC%
:: ---
if [%AUX_TC:~0,1%]==[v] call :get_msvc_version %AUX_TC%
:: ---
goto end_aux_env_check
:no_aux_env
set AUX_TC=%NOP%
::
:end_aux_env_check
:: when PRI_TC set to echo. version already known
if [%PRI_TC:~0,1%]==[g] call :get_gcc_version %PRI_TC%
:: ---
if [%PRI_TC:~0,1%]==[v] call :get_msvc_version %PRI_TC%
:: ---
::
set MACROS=
if defined CC_STUB_VC_VER set MACROS=-DCC_STUB_VC_VER=%CC_STUB_VC_VER%
if defined CC_STUB_GC_VER set MACROS=-DCC_STUB_GC_VER=%CC_STUB_GC_VER%
if defined CC_STUB_TS set "MACROS=%MACROS% -DCC_STUB_TS=%CC_STUB_TS%
set MACROS=-DCC_STUB_CONFIG=^"%OPTS%^" %MACROS%
:: echo %MACROS%
 echo cmd /d /c "%PRI_TC% & %AUX_TC% & %CCMD% %MACROS% %ARGS%
cmd /d /c "%PRI_TC% >nul & %AUX_TC% >nul & %CCMD% %MACROS% %ARGS% 2>&1"
goto :eof

:distclean
del /a *.lib 2> nul
del /a *.dll 2> nul
del /a *.exe 2> nul
del /a *.exe.manifest > nul 2> nul

:clean
del /a/f *.o > nul 2> nul
del /a/f *.obj > nul 2> nul
del /a/f *.out > nul 2> nul
del /a/f *.suo > nul 2> nul
del /a/f *.exp > nul 2> nul
del /a/f *.ilk > nul 2> nul
del /a/f *.pdb > nul 2> nul
del /a/f *.plist > nul 2> nul
del /a/f *.nativecodeanalysis.xml > nul 2> nul
goto :eof

:error_incomplete_env
echo no-file:0:0: error: compiler not found, perhaps incomplete environment
goto usage

:error_command_id
echo no-file:0:0: error: %NAME%: incorrect command-id
goto usage

:error_lint_bat
echo no-file:0:0: error: missing code-analysis batch script '%PRI_TC%'
goto usage

:error_env_bat
echo no-file:0:0: error: missing compiler environment setup script '%PRI_TC%'
goto usage

:usage
echo.Try '%NAME% ^<command-id^> [c++ file] [options...]'
echo.command-id can be one of: 0 1 11 2 3 31 5 7 b c e f
echo.  0:  clean        similar to 'make clean'
echo.
echo.  1:  Cpplint      github.com/cpplint
echo.  11  Flint++      github.com/L2Program/FlintPlusPlus
echo.  2:  Cppcheck     cppcheck.sourceforge.net/
echo.  3:  PMD-CPD      pmd.github.io/
echo.  31: Sloppy       strlen.com/sloppy/
echo. 
echo.  5:  LLVM 5.x --- with default VC++ compiler
echo.  7:  GNU GCC 7.2 --- MinGW-w64 x86_64 posix-seh or i686 posix-dwarf
echo.  b:  MS VC++ 11.0 --- aka Visual C++ 2012
echo.  c:  MS VC++ 12.0 --- aka Visual C++ 2013
echo.  e:  MS VC++ 14.0 --- aka Visual C++ 2015
echo.  f:  MS VC++ 14.1 --- aka Visual C++ 2017
echo. 
echo.  @  checks the PATH for clang++/g++/cl and uses that
::
exit /b 1
goto :eof

:get_args
:: removing 'command-id' argument---i.e. CID
set ARGS=%*
set A0=%1x
set A0=%A0:"=x%
set A0=%A0: =x%
:gobble
if [%A0%]==[] goto end_gobble
set ARGS=%ARGS:~1%
set A0=%A0:~1%
goto gobble
:end_gobble
exit /b 0

:process_args
:: after removing 'command-id' argument---i.e. CID
:: 1. collects compiler options only - i.e. no files
:: 2. removing duplicate -EH -W* options --- although cl uses the last
:: NOTE -W* and -EH[acs] removal is relevant only for VC++
set EH=-EHsc
::set ARGS=
set WARN=
::
:check_next_arg
shift
if [%1]==[] goto end_of_arg_check
set AA=%1
if exist %1 call set OPTS=%%OPTS:!AA!={SOURCE}%%
set AA=%AA:"=%
set AA=%AA: =x%
if [%AA:~0,2%]==[-W] set WARN=%AA%
if [%AA:~0,3%]==[-EH] set EH=
::set ARGS=%ARGS% %1
::if not exist %1 set COPTS=%COPTS% %1
goto check_next_arg
:end_of_arg_check
::set OPTS=%OPTS:  = %
exit /b 0

:where_is
:: NOTE the argument should have extension, except "echo."
if "%1"=="%NOP%" exit /b 0
if exist %1 exit /b 0
for %%i in (%1) do if NOT "%%~$PATH:i"=="" exit /b 0
exit /b 1

:get_gcc_version
if [%1]==[] goto exit /b 1
:: gnu/g++ option -dumpversion prints just one line only with the version
set "CC_STUB_GC_VER="
for /f %%i in ('%1 ^>nul ^& g++ -dumpversion 2^>^&1') do (
  set "CC_STUB_GC_VER=%%i"
)
:: TODO -dumpmachine with cross-compilers 
for /f %%i in ('%1 ^>nul ^& g++ -dumpmachine 2^>^&1') do (
  set "CC_STUB_GC_VER=%CC_STUB_GC_VER%/%%i"
)
exit /b 0

:get_msvc_version
if [%1]==[] goto exit /b 1
:: vc++/cl prints several lines and the first line has the version
:: it seems that for 32-bit compilers 8th token is the version
:: otherwise (for 64-bit compilers) 7th token is the version

for /f "tokens=7,8" %%i in ('%1 ^>nul ^& cl 2^>^&1') do (
  if not defined CC_STUB_VC_VER (
    if "%%i"=="Version" ( set "CC_STUB_VC_VER=%%j" ) else ( set "CC_STUB_VC_VER=%%i" )
  )
)
exit /b 0

goto :eof
:: end-of-file
