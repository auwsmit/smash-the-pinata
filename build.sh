#!/usr/bin/env bash
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Credit to RAD Debugger for many pointers and ideas

# Usage Notes
# -----------------------------------------------------------------------------
#
# This is the build script for this project on Linux.
#
# It can be used in two main ways:
# 1. `./build` to simply compile the game with a C compiler
# 2. `./build cmake` to setup and run the CMake build system
# Either way, you will find the output executable in the project directory.
#
# Additional arguments can be given to choose the compiler, compiler settings,
# or to clean up old build files.
#
# Below is a non-exhaustive list of arguments you can use:
# `build release` -> optimized build, no debug symbols
# `build clang`   -> use clang compiler
# `build web`     -> compile to web assembly with emscripten
# `build clean`   -> delete old generated build files (excluding CMakefi
# CMake build:
# `build cmake`       -> setup and build using CMake
# `build cmake web`   -> compile to web assembly with emscripten
# `build cmake clean` -> delete CMake's old build files
#
# Note: CMake will automatically download and build raylib if needed
#
# -----------------------------------------------------------------------------

# Project Config
# -----------------------------------------------------------------------------
output=slapmaster
cmake_build_dir=build
source_dir=src
source_code=
for f in "$script_dir/$source_dir"/*.c; do source_code="$source_code \"$f\""; done
for f in "$script_dir/$source_dir/entity"/*.c; do source_code="$source_code \"$f\""; done

# Script Entry Point
main()
{
    script_unpack_args "$@"
    pushd "$script_dir" > /dev/null
    if [[ "$cmake" == 1 ]]; then
        script_choose_cmake_lines
        script_cmake_config_and_build
    else
        script_choose_simple_lines
        script_simple_build
    fi
    popd
}

# Unpack Arguments
script_unpack_args()
{
    for arg in "$@"; do eval "$arg=1"; done
    if [[ "$clean" == 1 ]]; then
        echo "[clean mode]" && script_build_cleanup
        exit 0
    fi
    if [[ "$web" != 1     ]]; then release=1; fi
    if [[ "$release" != 1 ]]; then debug=1; fi
    if [[ "$release" == 1 ]]; then debug=0 && echo "[release mode]"; fi
    if [[ "$debug" == 1   ]]; then release=0 && echo "[debug mode]"; fi
    if [[ "$cmake" == 1   ]]; then
        echo "[cmake build]"
        if [[ "$web" != 1 ]]; then gcc=1 && echo "[gcc compile]"; fi
    else
        simple_build=1 && echo "[simple build]"
        if [[ "$clang" == 1 ]]; then gcc=0 && echo "[clang compile]"; fi
        if [[ "$web" == 1   ]]; then clang=0; fi
        if [[ "$web" != 1 && "$clang" != 1 ]]; then
            gcc=1 && echo "[gcc compile]"
        fi
    fi
    if [[ "$web" == 1 ]]; then echo "[web compile]"; fi
}

# Define and Choose CMake Lines
script_choose_cmake_lines()
{
    # Line Definitions
    cmake_setup_desktop="cmake -DOUTPUT_NAME=$output -B \"$cmake_build_dir/desktop\" -DPLATFORM=Desktop"
    cmake_build_desktop="cmake --build \"$cmake_build_dir/desktop\""
    cmake_setup_web="emcmake cmake -DOUTPUT_NAME=$output -DCMAKE_EXECUTABLE_SUFFIX=\".html\" -B \"$cmake_build_dir/web\" -DPLATFORM=Web"
    cmake_build_web="emmake make -C \"$cmake_build_dir/web\""

    # Choose Lines
    if [[ "$web" == 1 ]]; then
        output_dir=$cmake_build_dir/web
        cmake_setup_cmd=$cmake_setup_web
        cmake_build_cmd=$cmake_build_web
    else
        output_dir=$cmake_build_dir/desktop
        cmake_setup_cmd=$cmake_setup_desktop
        cmake_build_cmd=$cmake_build_desktop
    fi
    if [[ "$release" == 1 ]]; then cmake_setup_flags='-DCMAKE_BUILD_TYPE=Release'; fi
    if [[ "$debug" == 1 ]]; then cmake_setup_flags='-DCMAKE_BUILD_TYPE=Debug'; fi
}

# Define and Choose Compile/Link Lines
script_choose_simple_lines()
{
    # Line Definitions
    cc_common="-I\"raylib/include\" -I\"${source_dir}/include\" -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wfloat-conversion"
    cc_debug='-g -O0'
    cc_release='-O2'
    cc_platform='-DPLATFORM_DESKTOP'
    cc_link='-lraylib -lGL -lm -lpthread -ldl -lrt -lX11'
    cc_out='-o'

    web_release='-Os'
    web_platform='-DPLATFORM_WEB'
    web_link='-lraylib -L"raylib/lib/web" --shell-file shell.html -sUSE_GLFW=3 -sTOTAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 -sASYNCIFY -sEXPORTED_FUNCTIONS=_main,requestFullscreen -sEXPORTED_RUNTIME_METHODS=HEAPF32 preload-file assets'

    # Choose Lines
    if [[ "$gcc" == 1     ]]; then compile="gcc $cc_common"; fi
    if [[ "$clang" == 1   ]]; then compile="clang $cc_common"; fi
    if [[ "$web" == 1     ]]; then compile="emcc $cc_common"; fi
    if [[ "$web" == 1     ]]; then compile_platform="$web_platform"; fi
    if [[ "$web" != 1     ]]; then compile_platform="$cc_platform"; fi
    if [[ "$web" == 1     ]]; then compile_link="$web_link"; fi
    if [[ "$web" != 1     ]]; then compile_link="$cc_link"; fi
    if [[ "$web" == 1     ]]; then compile_out="$cc_out $output.html"; fi
    if [[ "$web" != 1     ]]; then compile_out="$cc_out $output"; fi
    if [[ "$web" == 1     ]]; then compile_release="$web_release"; fi
    if [[ "$debug" == 1   ]]; then compile="$compile $cc_debug"; fi
    if [[ "$release" == 1 ]]; then compile="$compile $cc_release"; fi
    compile="$compile $compile_platform"
}

script_cmake_config_and_build()
{
    echo "$cmake_setup_cmd $cmake_setup_flags"
    echo "$cmake_build_cmd $cmake_build_flags"
    eval $cmake_setup_cmd $cmake_setup_flags
    eval $cmake_build_cmd $cmake_build_flags
    if [[ "$web" == 1 ]]; then
        rm -f "$output.html" "$output.js" "$output.wasm" "$output.data"
        cp "$output_dir/$output.html" .
        cp "$output_dir/$output.js" .
        cp "$output_dir/$output.wasm" .
        cp "$output_dir/$output.data" .
    else
        rm -f "$output"
        cp "$output_dir/$output" .
    fi
}

script_simple_build()
{
    echo "$compile $source_code $compile_out $compile_link"
    eval $compile $source_code $compile_out $compile_link
}

script_build_cleanup()
{
    pushd "$script_dir" > /dev/null
    if [[ "$cmake" == 1 ]]; then
        rm -rf $output build/
        echo "CMake build files cleaned"
    else
        rm -rf $output build_web/ $output.html $output.js $output.wasm $output.data
        echo "Build files cleaned"
    fi
    popd
}

main "$@"
