#!/bin/bash
set -e
export PATH=/snap/bin:$PATH

# =============================
# CONFIG
# =============================
APP_NAME="PlantSim"
BIN_NAME="plantsim.exe"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

SRC="$ROOT/src"
LIBS="$ROOT/libs"
ASSETS="$ROOT/assets"

BUILD="$ROOT/build/windows"
OBJ="$BUILD/obj"
DIST="$ROOT/dist/windows"
TOOLS="$ROOT/tools"

RAYLIB_BUILD="$LIBS/raylib/build-mingw"

CC=x86_64-w64-mingw32-gcc
CXX=x86_64-w64-mingw32-g++
MAKE=make

mkdir -p "$OBJ" "$DIST" "$TOOLS"

info()    { echo "-> $*"; }
success() { echo "-> $*"; }
warn()    { echo "-> $*"; }

# ==============================================================================
# 1) DEPENDENCIES
# ==============================================================================
echo "-> Running dependency setup..."
echo "-> Ensuring dependencies are present..."
./scripts/clone-libs.sh
echo "-> Dependency check complete"

# ==============================================================================
# 2) RAYLIB
# ==============================================================================
RAYLIB_LIB=$(find "$RAYLIB_BUILD" -name "libraylib.a" | head -n 1)

if [ -z "$RAYLIB_LIB" ]; then
    info "Building raylib statically..."
    rm -rf "$RAYLIB_BUILD"
    mkdir -p "$RAYLIB_BUILD"
    pushd "$RAYLIB_BUILD" >/dev/null

    cmake .. \
        -DCMAKE_SYSTEM_NAME=Windows \
        -DCMAKE_C_COMPILER=$CC \
        -DCMAKE_CXX_COMPILER=$CXX \
        -DBUILD_SHARED_LIBS=OFF \
        -DPLATFORM=Desktop \
        -DBUILD_EXAMPLES=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$RAYLIB_BUILD"

    $MAKE -j$(nproc)
    popd >/dev/null

    if [ ! -f "$RAYLIB_BUILD/libraylib.a" ]; then
        echo "error: libraylib.a not found after build"
        exit 1
    fi
else
    info "raylib already built"
fi

# ==============================================================================
# 3) IMGUI & RLMGUI
# ==============================================================================
info "Compiling ImGui and rlImGui..."

CXXFLAGS="-std=c++17 -O2 -I$LIBS/imgui -I$LIBS/rlImGui -I$LIBS/raylib/src -I$SRC"

IMGUI_SRC=(
    "$LIBS/imgui/imgui.cpp"
    "$LIBS/imgui/imgui_draw.cpp"
    "$LIBS/imgui/imgui_tables.cpp"
    "$LIBS/imgui/imgui_widgets.cpp"
)

RLIMGUI_SRC=(
    "$LIBS/rlImGui/rlImGui.cpp"
)

for src in "${IMGUI_SRC[@]}" "${RLIMGUI_SRC[@]}"; do
    obj="$OBJ/$(basename "$src" .cpp).o"
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        info "$src"
        $CXX -c "$src" $CXXFLAGS -o "$obj"
    else
        info "$src up to date"
    fi
done

# ==============================================================================
# 4) PROJECT CLEANUP
# ==============================================================================
info "Cleaning project object files"

if [ -d "$OBJ" ]; then
    find "$SRC" -name "*.cpp" -exec basename {} .cpp \; | while read -r base; do
        if [ -f "$OBJ/$base.o" ]; then
            info "Removing $OBJ/$base.o"
            rm -f "$OBJ/$base.o"
        fi
    done
fi

# ==============================================================================
# 5) COMPILE SOURCES
# ==============================================================================
info "Compiling objects"

SRC_CPP=($(find "$SRC" -name "*.cpp"))

for src in "${SRC_CPP[@]}"; do
    obj="$OBJ/$(basename "$src" .cpp).o"
    mkdir -p "$(dirname "$obj")"
    info "Compiling $src -> $obj"
    $CXX -c "$src" $CXXFLAGS -o "$obj"
done

# ==============================================================================
# 6) LINK
# ==============================================================================
info "Linking Windows executable"

OBJ_FILES=($(find "$OBJ" -name "*.o"))
OUT="$DIST/$BIN_NAME"

$CXX "${OBJ_FILES[@]}" $CXXFLAGS \
    "$RAYLIB_BUILD/libraylib.a" \
    -lopengl32 -lgdi32 -lwinmm -lole32 -luser32 -lkernel32 \
    -static -static-libgcc -static-libstdc++ \
    -o "$OUT"

success "Windows binary ready: $OUT"

# ==============================================================================
# 7) ASSETS
# ==============================================================================
info "Copying assets"
mkdir -p "$DIST/assets"
cp -r "$ASSETS"/* "$DIST/assets/"

success "Assets copied to $DIST/assets"