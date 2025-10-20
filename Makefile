# Credit to raylib's makefiles and the RAD Debugger for many pointers and ideas

# --- Usage Notes -------------------------------------------------------------
#
# This makefile is used to build this project on Windows and Linux.
#
# With no arguments, running `make` will build the game executable for Desktop
# with gcc and place a copy in the repo directory. It can take a single
# argument that specifies which compiler to use.
#
# Below is a list of arguments you can use:
# `make CONFIG=RELEASE`  -> optimized build, no debug files (debug is default)
# `make msvc`  --> use msvc/cl.exe to compile
# `make web`   --> compile to web assembly with emscripten
# `make clean` --> delete all previously generated build files
#
# -----------------------------------------------------------------------------

# =============================================================================
# Project Config
# =============================================================================

# Output executable name
OUTPUT  := SmashThePinata

# Source code, headers, and object file paths
SRC_DIR := src
INC_DIR := $(SRC_DIR)/include
HEADERS := $(wildcard $(INC_DIR)/*.h)
SRC     := $(wildcard $(SRC_DIR)/*.c)

# =============================================================================
# Platform Settings
# =============================================================================

ifeq ($(PLATFORM),WEB)
    OUTPUT := index
    EXTENSION := .html
else ifeq ($(OS),Windows_NT)
    OS := WINDOWS
    EXTENSION := .exe
else
    ifeq ($(shell uname -s),Linux)
        OS := LINUX
    else ifeq ($(shell uname -s),Darwin)
        OS := MAC
    endif
    EXTENSION :=
endif

# =============================================================================
# Compiler Settings
# =============================================================================

# Compiler fallback just in case
ifeq ($(PLATFORM),WEB)
    CC := emcc
else
    CC ?= gcc
endif

# Debug build by default
CONFIG ?= DEBUG

# Optimization / Debug flags
ifeq ($(CC),cl) # MSVC
    ifeq ($(CONFIG),RELEASE)
        OPTIMIZE_FLAGS := /O2
    else ifeq ($(CONFIG),DEBUG)
        OPTIMIZE_FLAGS := /Od /Zi
    endif
else ifeq ($(PLATFORM),WEB) # (TODO check emscripten debug page)
    OPTIMIZE_FLAGS := -Os
else ifeq ($(CONFIG),RELEASE)
    OPTIMIZE_FLAGS := -O2
else ifeq ($(CONFIG),DEBUG)
    OPTIMIZE_FLAGS := -g -O0
endif

# Define C compiler flags
# -----------------------------------------------------------------------------
#  -O1                  defines optimization level
#  -g                   include debug information on compilation
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -Wno-unused-value    ignore unused return values of some functions (i.e. fread())
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
CFLAGS := -std=c99 -Wall -Wno-missing-braces -Wunused-result
# Additional flags for compiler (if desired)
#  -Wextra                enables some extra warning flags that are not enabled by -Wall
#  -Wmissing-prototypes   warn if a global function is defined without a previous prototype declaration
#  -Wstrict-prototypes    warn if a function is declared or defined without specifying the argument types
#  -Wfloat-conversion     warn about implicit type conversion for floats
#  -Werror=implicit-function-declaration    catch function calls without prior declaration
CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wfloat-conversion

# MSVC cl.exe Flags
# -----------------------------------------------------------------------------
# /W3    Set warning level to 3 (default is 1, max is 4)
# /MD    Link against MSVCRT.DLL (multithreaded DLL runtime)
ifeq ($(CC),cl)
    CFLAGS := /W3 /MD
endif

# Define C preprocessor flags and linker flags
CPPFLAGS  := -I"raylib/include" -I"$(INC_DIR)" -D_DEFAULT_SOURCE
PLATFORM_FLAG  := -DPLATFORM_DESKTOP
ifeq ($(CC),cl)
    CPPFLAGS := /I"raylib/include" /I"$(INC_DIR)"
    LINKFLAGS := /link /LIBPATH:"raylib/lib/windows-msvc" \
                  raylib.lib gdi32.lib winmm.lib user32.lib shell32.lib
    ifeq ($(CONFIG),DEBUG)
        LINKFLAGS += /DEBUG
    endif
    PLATFORM_FLAG := /DPLATFORM_DESKTOP
else ifeq ($(OS),WINDOWS)
    LINKFLAGS  := -lraylib -L"raylib/lib/windows" -lopengl32 -lgdi32 -lwinmm
else ifeq ($(OS),LINUX)
    LINKFLAGS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else ifeq ($(OS),MAC)
    LINKFLAGS  := -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif
ifeq ($(PLATFORM),WEB)
    # Web (emscripten emcc) Flags
    # -----------------------------------------------------------------------------
    # -Os                         size optimization
    # -sUSE_GLFW=3                Use glfw3 library (context/input management)
    # -sALLOW_MEMORY_GROWTH=1     to allow memory resizing -> WARNING: Audio buffers could FAIL!
    # -sTOTAL_MEMORY=16777216     to specify heap memory size (default = 16MB) (67108864 = 64MB)
    # -sUSE_PTHREADS=1            multithreading support
    # -sWASM=0                    disable Web Assembly, emitted by default
    # -sASYNCIFY                  lets synchronous C/C++ code interact with asynchronous JS
    # -sFORCE_FILESYSTEM=1        force filesystem to load/save files data
    # -sASSERTIONS=1              enable runtime checks for common memory allocation errors (-O1 and above turn it off)
    # -sGL_ENABLE_GET_PROC_ADDRESS  enable using the *glGetProcAddress() family of functions, required for extensions loading
    # -sEXPORTED_FUNCTIONS=       export needed functions (only for newer versions of emscripten to help reduce filesize)
    # -sEXPORTED_RUNTIME_METHODS= export runtime functions
    # --profiling                 include information for code profiling
    # --memory-init-file 0        to avoid an external memory initialization code file (.mem)
    # --preload-file resources    specify a resources folder for data compilation
    # --source-map-base           allow debugging in browser with source map
    LINKFLAGS  := -lraylib -L"raylib/lib/web" --shell-file shell.html \
    -sUSE_GLFW=3 -sFORCE_FILESYSTEM=1 -sASYNCIFY -sTOTAL_MEMORY=67108864 \
    -sEXPORTED_FUNCTIONS=_main,requestFullscreen -sEXPORTED_RUNTIME_METHODS=HEAPF32 \
    --preload-file assets
    PLATFORM_FLAG := -DPLATFORM_WEB
endif

# Define output flags
ifeq ($(CC),cl)
    NOLINK := /c
    OUT_FLAG := /Fe:$(OUTPUT)$(EXTENSION)
else
    NOLINK := -c
    OUT_FLAG := -o $(OUTPUT)$(EXTENSION)
endif

# Combine CFLAGS
CFLAGS += $(OPTIMIZE_FLAGS) $(CPPFLAGS) $(PLATFORM_FLAG)

# ==============================================================================
# Targets
# ==============================================================================

# let `make` know that these aren't files
.PHONY: all clang msvc web gh-pages clean

# Default: Compile all files for desktop
all:
	$(CC) $(CFLAGS) $(SRC) $(OUT_FLAG) $(LINKFLAGS)

# Build with clang
clang:
	$(MAKE) CC=clang

# Build with MSVC cl.exe
msvc:
	$(MAKE) CC=cl
	@rm -f *.obj

# Build to web assembly with emscripten
web:
	$(MAKE) PLATFORM=WEB

# Build for upload to GitHub pages
# (Automated by GitHub workflow: .github/workflows/deploy.yaml)
gh-pages:
	@mkdir -p build_web
	$(MAKE) PLATFORM=WEB OUTPUT=build_web/index

run:
	$(MAKE) && ./$(OUTPUT)$(EXTENSION)

# Clean up generated build files
clean:
	@rm -rf $(OUTPUT)$(EXTENSION) \
	        index.html index.js index.wasm index.data build_web/ \
	        $(OUTPUT).ilk $(OUTPUT).pdb vc140.pdb *.rdi
	@echo "Make build files cleaned"
