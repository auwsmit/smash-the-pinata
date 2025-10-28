# --- Usage Notes -------------------------------------------------------------
#
# This makefile is used to build this project on Windows, Linux, or Mac.
#
# With no arguments, running `make` will build the game for Desktop with gcc.
# The executable/output will be in the repo directory.
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

# Output name
ifeq ($(PLATFORM),WEB)
    OUTPUT := index
else
    OUTPUT := SmashThePinata
endif

# Paths to source code
SRC_DIR := src
INC_DIR := $(SRC_DIR)/include
HEADERS := $(wildcard $(INC_DIR)/*.h)
SRC     := $(wildcard $(SRC_DIR)/*.c)

# Debug build by default
CONFIG  ?= DEBUG

# Default compiler settings
OPTIMIZE_FLAGS := -O2
DEBUG_FLAGS    := -g -O0
CFLAGS         := -std=c99 -Wall -Wno-missing-braces -Wunused-result
CFLAGS         += -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wfloat-conversion
CPPFLAGS       := -I"raylib/include" -I"$(INC_DIR)" -D_DEFAULT_SOURCE
PLATFORM_DEF   := -DPLATFORM_DESKTOP

# OS and platform settings
EXTENSION      :=
ifeq ($(OS),Windows_NT)
    EXTENSION  := .exe
    LDFLAGS    := -lraylib -L"raylib/lib/windows" -lopengl32 -lgdi32 -lwinmm
else ifeq ($(shell uname -s),Linux)
    LDFLAGS    := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
else ifeq ($(shell uname -s),Darwin) # MacOS
    LDFLAGS    := -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif
LDFLAGS_DEBUG  :=
ifeq ($(PLATFORM),WEB)
    EXTENSION  := .html
    CC := emcc
else
    CC ?= gcc
endif
OUTPUT_FLAG := -o $(OUTPUT)$(EXTENSION)

# Compiler-specific overrides
ifeq ($(CC),cl)
    OPTIMIZE_FLAGS := /O2
    DEBUG_FLAGS    := /Od /Zi
    CFLAGS         := /W3 /MD
    LDFLAGS        := /link /LIBPATH:"raylib/lib/windows-msvc" \
                      raylib.lib gdi32.lib winmm.lib user32.lib shell32.lib
    LDFLAGS_DEBUG  := /DEBUG
    PLATFORM_DEF   := /DPLATFORM_DESKTOP
    OUTPUT_FLAG    := /Fe:$(OUTPUT)$(EXTENSION)
else ifeq ($(CC),emcc)
    OPTIMIZE_FLAGS := -Os
    DEBUG_FLAGS    := $(OPTIMIZE_FLAGS)
    LDFLAGS        := -lraylib -L"raylib/lib/web" --shell-file shell.html \
                      -sUSE_GLFW=3 -sFORCE_FILESYSTEM=1 -sASYNCIFY -sTOTAL_MEMORY=67108864 \
                      -sEXPORTED_FUNCTIONS=_main,requestFullscreen -sEXPORTED_RUNTIME_METHODS=HEAPF32 \
                      --preload-file assets
    PLATFORM_DEF   := -DPLATFORM_WEB
endif

# Debug or Release build
ifeq ($(CONFIG),DEBUG)
    CFLAGS += $(DEBUG_FLAGS)
    LDFLAGS += $(LDFLAGS_DEBUG)
else
    CFLAGS += $(OPTIMIZE_FLAGS)
endif

# Combine CFLAGS
CFLAGS += $(CPPFLAGS) $(PLATFORM_DEF)

# =============================================================================
# Targets
# =============================================================================

# let `make` know that these aren't files
.PHONY: all clang msvc web clean run

# Default: Compile all files for desktop
all:
	$(CC) $(CFLAGS) $(SRC) $(OUTPUT_FLAG) $(LDFLAGS)

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

run:
	$(MAKE) && ./$(OUTPUT)$(EXTENSION)

# Clean up generated build files
clean:
	@rm -rf $(OUTPUT)$(EXTENSION) \
	        index.html index.js index.wasm index.data \
	        $(OUTPUT).ilk $(OUTPUT).pdb vc140.pdb *.rdi
	@echo "Make build files cleaned"
