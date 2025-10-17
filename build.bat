@echo off
setlocal enabledelayedexpansion
set script_dir=%~dp0

:: Credit to RAD Debugger for many pointers and ideas

:: Usage Notes
:: ----------------------------------------------------------------------------
::
:: This is the build script for this project on Windows.
::
:: It can be used in two main ways:
:: 1. `.\build` to simply compile the game with a C compiler
:: 2. `.\build cmake` to setup and run the CMake build system
:: Either way, you will find the output executable in the project directory.
::
:: Additional arguments can be given to choose the compiler, compiler settings,
:: or to clean up old build files.
::
:: Below is a non-exhaustive list of arguments you can use:
:: `build release` -> optimized build, no debug symbols
:: `build clang`   -> use clang compiler
:: `build msvc`    -> use msvc compiler
:: `build web`     -> compile to web assembly with emscripten
:: `build clean`   -> delete old generated build files (excluding CMake)
:: CMake build:
:: `build cmake`       -> setup and build using CMake
:: `build cmake gcc`   -> use MinGW compiler instead of MSVC
:: `build cmake web`   -> compile to web assembly with emscripten
:: `build cmake clean` -> delete CMake's old build files
::
:: Notes for CMake:
:: - CMake will automatically download and build raylib if needed
:: - You can find the generated Visual Studio solution in build\desktop\
::
:: ----------------------------------------------------------------------------

:: Project Config
:: ----------------------------------------------------------------------------
set output=slapmaster
set cmake_build_dir=build
set source_dir=src
set source_code=
for %%f in ("%script_dir%%source_dir%\*.c") do set source_code=!source_code! "%%f"
for %%f in ("%script_dir%%source_dir%\entity\*.c") do set source_code=!source_code! "%%f"

:: Unpack Arguments
:: ----------------------------------------------------------------------------
for %%a in (%*) do set "%%a=1"
if "%clean%"=="1"       echo [clean mode] && goto :clean
if "%web%"=="1"         set "release=1"
if not "%release%"=="1" set "debug=1"
if "%release%"=="1"     set "debug=0" && echo [release mode]
if "%debug%"=="1"       set "release=0"   && echo [debug mode]
if "%cmake%"=="1" (
    echo [cmake build]
    if "%gcc%"=="1" echo [gcc compile]
    if not "%web%"=="1" if not "%gcc%"=="1" (
        set "msvc=1" && echo [msvc compile]
    )
) else (
    set "simple_build=1" && echo [simple build]
    if "%clang%"=="1" set "msvc=0" && echo [clang compile]
    if "%msvc%"=="1"  set "clang=0" && echo [msvc compile]
    if "%web%"=="1"   set "clang=0" && set "msvc=0"
    if not "%clang%"=="1" if not "%msvc%"=="1" if not "%web%"=="1" (
        set "gcc=1" && echo [gcc compile]
    )
)
if "%web%"=="1" echo [web compile]

:: CMake Line Definitions
:: ----------------------------------------------------------------------------
if "%cmake%"=="1" (
    set cmake_setup_desktop=cmake -DOUTPUT_NAME=%output% -B "%cmake_build_dir%\desktop" -DPLATFORM=Desktop
    set cmake_build_desktop=cmake --build "%cmake_build_dir%\desktop"
    set cmake_setup_web=emcmake cmake -DOUTPUT_NAME=%output% -DCMAKE_EXECUTABLE_SUFFIX=".html" -B "%cmake_build_dir%\web" -DPLATFORM=Web
    set cmake_build_web=emmake make -C "%cmake_build_dir%\web"
)

:: Choose CMake Lines
:: ----------------------------------------------------------------------------
if "%cmake%"=="1"   (
    if "%web%"=="1" (
        set output_dir=%cmake_build_dir%\web
        set cmake_setup_cmd=%cmake_setup_web%
        set cmake_build_cmd=%cmake_build_web%
    ) else (
        set cmake_setup_cmd=%cmake_setup_desktop%
        set cmake_build_cmd=%cmake_build_desktop%
    )
    if "%msvc%"=="1" (
        if "%release%"=="1" (
            set cmake_build_flags=--config Release
            set output_dir=%cmake_build_dir%\desktop\Release
        ) else (
            set cmake_build_flags=--config Debug
            set output_dir=%cmake_build_dir%\desktop\Debug
        )
    ) else (
        set cmake_setup_flags=-G "MinGW Makefiles"
        if "%release%"=="1" set cmake_setup_flags=-DCMAKE_BUILD_TYPE=Release %cmake_setup_flags%
        if "%debug%"=="1"   set cmake_setup_flags=-DCMAKE_BUILD_TYPE=Debug %cmake_setup_flags%
    )
)

:: Compile/Link Line Definitions
:: ----------------------------------------------------------------------------
set cc_common=   -I"raylib\include" -I"%source_dir%\include" -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wfloat-conversion
set cc_debug=    -g -O0
set cc_release=  -O2
set cc_platform= -DPLATFORM_DESKTOP
set cc_link=     -lraylib -L"raylib\lib\windows" -lopengl32 -lgdi32 -lwinmm
set cc_out=      -o

set cl_common=   cl /I"raylib\include" /I"%source_dir%\include" /W3 /MD
set cl_debug=    /Od /Zi
set cl_release=  /O2
set cl_platform= /DPLATFORM_DESKTOP
set cl_link=     /link /INCREMENTAL:NO /LIBPATH:"raylib\lib\windows-msvc" raylib.lib gdi32.lib winmm.lib user32.lib shell32.lib
set cl_link_debug= /DEBUG
set cl_out=      /Fe:

set web_release=  -Os
set web_platform= -DPLATFORM_WEB
set web_link=     -lraylib -L"raylib\lib\web" --shell-file shell.html -sUSE_GLFW=3 -sTOTAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 -sASYNCIFY -sEXPORTED_FUNCTIONS=_main,requestFullscreen -sEXPORTED_RUNTIME_METHODS=HEAPF32 --preload-file assets

:: Choose Compile/Link Lines
:: ----------------------------------------------------------------------------
if     "%gcc%"=="1"   set compile=gcc %cc_common%
if     "%clang%"=="1" set compile=clang %cc_common%
if     "%web%"=="1"   set compile=emcc %cc_common%
if     "%msvc%"=="1"  set compile=%cl_common%
if     "%web%"=="1"   set compile_platform=%web_platform%
if not "%web%"=="1"   set compile_platform=%cc_platform%
if     "%web%"=="1"                      set compile_link=%web_link%
if not "%web%"=="1" if not "%msvc%"=="1" set compile_link=%cc_link%
if     "%web%"=="1"                      set compile_out=%cc_out% %output%.html
if not "%web%"=="1" if not "%msvc%"=="1" set compile_out=%cc_out% %output%.exe

if "%debug%"=="1"    set cl_link=%cl_link% %cl_link_debug%
if "%msvc%"=="1"     set compile_link=%cl_link%
if "%msvc%"=="1"     set compile_out=%cl_out%%output%.exe
if "%msvc%"=="1"     set compile_platform=%cl_platform%

if not "%msvc%"=="1" set compile_debug=%cc_debug%
if     "%msvc%"=="1" set compile_debug=%cl_debug%
if not "%msvc%"=="1" set compile_release=%cc_release%
if     "%msvc%"=="1" set compile_release=%cl_release%
if     "%web%"=="1"  set compile_release=%web_release%

if "%debug%"=="1"    set compile=%compile% %compile_debug%
if "%release%"=="1"  set compile=%compile% %compile_release%
set compile=%compile% %compile_platform%

:: Build The Project
:: ----------------------------------------------------------------------------
pushd "%script_dir%"
if "%cmake%"=="1" (
    echo %cmake_setup_cmd% %cmake_setup_flags%
    echo %cmake_build_cmd% %cmake_build_flags%
    %cmake_setup_cmd% %cmake_setup_flags%
    %cmake_build_cmd% %cmake_build_flags%
    if "%web%"=="1" (
        del /q "%output%.html" "%output%.js" "%output%.wasm" "%output%.data"
        copy "%output_dir%\%output%.html" .
        copy "%output_dir%\%output%.js" .
        copy "%output_dir%\%output%.wasm" .
        copy "%output_dir%\%output%.data" .
    ) else (
        del /q "%output%.exe"
        copy "%output_dir%\%output%.exe" .
    )
)

if "%simple_build%"=="1" (
    echo %compile% %source_code% %compile_out% %compile_link%
    %compile% %source_code% %compile_out% %compile_link%
    if "%msvc%"=="1" del /q "%script_dir%\*.obj"
)
popd

:: Clean Up
:: ----------------------------------------------------------------------------
:clean
if not "%clean%"=="1" goto :eof
pushd "%script_dir%"
if "%cmake%"=="1" (
    rmdir /s /q build
    del /q %output%.exe
    echo CMake build files cleaned
) else (
    rmdir /s /q build_web
    del /q %output%.exe
    del /q %output%.html %output%.js %output%.wasm %output%.data
    del /q %output%.ilk %output%.pdb vc140.pdb *.rdi
    echo Build files cleaned
)
popd
