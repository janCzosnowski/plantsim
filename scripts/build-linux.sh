#!/bin/bash
set -e
export PATH=/snap/bin:$PATH

# ==============================================================================
# CONFIGURATION
# ==============================================================================
APP_NAME="PlantSim"
BIN_NAME="plantsim"
APPIMAGE_NAME="${APP_NAME}-x86_64.AppImage"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/src"
LIBS="$ROOT/libs"
ASSETS="$ROOT/assets"
BUILD="$ROOT/build/linux"
OBJ="$BUILD/obj"
DIST="$ROOT/dist/linux"
TOOLS="$ROOT/tools"
APPDIR="$BUILD/AppDir"

# Dependency Build Paths
RAYLIB_BUILD="$LIBS/raylib/build"
FFMPEG_INSTALL_PATH="$LIBS/ffmpeg/build"

mkdir -p "$OBJ" "$DIST" "$TOOLS"

# ==============================================================================
# 1️⃣ ENSURE DEPENDENCIES
# ==============================================================================
echo "🔹 Running dependency setup..."
./scripts/clone-libs.sh

# ==============================================================================
# 2️⃣ BUILD RAYLIB 
# ==============================================================================
RAYLIB_LIB=$(find "$RAYLIB_BUILD" -name "libraylib.a" | head -n 1)

if [ -z "$RAYLIB_LIB" ]; then
    echo "🔹 Building raylib statically..."
    mkdir -p "$RAYLIB_BUILD"
    pushd "$RAYLIB_BUILD"
    cmake .. -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release \
             -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    make -j$(nproc)
    popd
    RAYLIB_LIB=$(find "$RAYLIB_BUILD" -name "libraylib.a" | head -n 1)
else
    echo "🔹 raylib found: $RAYLIB_LIB"
fi

# ==============================================================================
# 3️⃣ BUILD FFmpeg (STATIC)
# ==============================================================================
if [ ! -f "$FFMPEG_INSTALL_PATH/lib/libavformat.a" ]; then
    echo "🔹 Building FFmpeg..."
    mkdir -p "$FFMPEG_INSTALL_PATH"
    pushd "$LIBS/ffmpeg"
    ./configure \
        --prefix="$FFMPEG_INSTALL_PATH" \
        --enable-static --enable-pic --disable-shared \
        --disable-doc --disable-programs --disable-network \
        --disable-everything \
        --enable-avcodec --enable-avformat --enable-avutil \
        --enable-swscale --enable-swresample \
        --enable-decoder=h264 \
        --enable-parser=h264 \
        --enable-demuxer=mov,mp4 \
        --enable-protocol=file \
        --disable-vaapi \
        --disable-vdpau \
        --disable-vulkan \
        --disable-hwaccels \
        --disable-bzlib
    make -j$(nproc)
    make install
    popd
else
    echo "🔹 FFmpeg found: $FFMPEG_INSTALL_PATH"
fi

# ==============================================================================
# 4️⃣ COMPILATION SETUP
# ==============================================================================
CXXFLAGS="-std=c++17 -O2"
CXXFLAGS+=" -fno-builtin-sscanf"
CXXFLAGS+=" -isystem $FFMPEG_INSTALL_PATH/include"
CXXFLAGS+=" -I$LIBS/imgui -I$LIBS/rlImGui -I$LIBS/raylib/src -I$SRC"
CXXFLAGS+=" -DPLATFORM_LINUX -Wall -Wextra"

# ==============================================================================
# 5️⃣ PROJECT CLEANUP (Fixed logic)
# ==============================================================================
echo "🔹 Cleaning project object files..."
# This cleans object files compiled from
# source files in src/
if [ -d "$OBJ" ]; then
    # Find all .cpp files in src/, get their names, and delete corresponding .o
    find "$SRC" -name "*.cpp" -exec basename {} .cpp \; | while read -r base; do
        if [ -f "$OBJ/$base.o" ]; then
            echo "   -> Removing $OBJ/$base.o"
            rm -f "$OBJ/$base.o"
        fi
    done
fi

# ==============================================================================
# 6️⃣ COMPILE EVERYTHING
# ==============================================================================
echo "🔹 Compiling objects..."
SRC_CPP=($(find "$SRC" -name "*.cpp"))
LIB_SRCS=(
    "$LIBS/imgui/imgui.cpp" "$LIBS/imgui/imgui_draw.cpp" 
    "$LIBS/imgui/imgui_tables.cpp" "$LIBS/imgui/imgui_widgets.cpp" 
    "$LIBS/rlImGui/rlImGui.cpp"
)

# Compile Libraries first (only if missing)
for src in "${LIB_SRCS[@]}"; do
    obj="$OBJ/$(basename "$src" .cpp).o"
    if [ ! -f "$obj" ]; then
        echo "   -> lib: $(basename "$src")"
        g++ -c "$src" $CXXFLAGS -o "$obj"
    fi
done

# Compile source files from src/
for src in "${SRC_CPP[@]}"; do
    obj="$OBJ/$(basename "$src" .cpp).o"
    echo "   -> src: $(basename "$src")"
    g++ -c "$src" $CXXFLAGS -o "$obj"
done

# ==============================================================================
# 7️⃣ LINKING & PACKAGING
# ==============================================================================
echo "🔹 Linking $BIN_NAME..."
OUT_BIN="$BUILD/out/$BIN_NAME"
mkdir -p "$(dirname "$OUT_BIN")"
OBJ_FILES=($(find "$OBJ" -name "*.o"))

g++ "${OBJ_FILES[@]}" -o "$OUT_BIN" \
    "$RAYLIB_LIB" \
    "$FFMPEG_INSTALL_PATH/lib/libavformat.a" \
    "$FFMPEG_INSTALL_PATH/lib/libavcodec.a" \
    "$FFMPEG_INSTALL_PATH/lib/libswscale.a" \
    "$FFMPEG_INSTALL_PATH/lib/libswresample.a" \
    "$FFMPEG_INSTALL_PATH/lib/libavutil.a" \
    -L/usr/lib/x86_64-linux-gnu \
    -lGL -lX11 -lpthread -lm -ldl -lrt -lz -llzma -ldrm

# ==============================================================================
# 8️⃣ PACKAGING
# ==============================================================================
echo "🔹 Preparing AppDir..."
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/$APP_NAME/assets"

# 1️⃣ Copy Binary and Assets
cp "$OUT_BIN" "$APPDIR/usr/bin/"
cp -r "$ASSETS"/* "$APPDIR/usr/share/$APP_NAME/assets/"

# 2️⃣ Create the Desktop File
cat > "$APPDIR/$BIN_NAME.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
Exec=$BIN_NAME
Icon=$BIN_NAME
Categories=Game;Simulation;
Terminal=false
EOF

# 3️⃣ Prepare the Icon
DEPLOY_ICON="$ROOT/$BIN_NAME.png"
ICON="$DEPLOY_ICON"

if [ -f "$ASSETS/gfx/icon.png" ]; then
    cp "$ASSETS/gfx/icon.png" "$DEPLOY_ICON"
else
    echo "⚠️ Icon missing at $ASSETS/gfx/icon.png, creating fallback..."
    if command -v convert >/dev/null 2>&1; then
        convert -size 512x512 xc:green "$DEPLOY_ICON"
    else
        # Minimal valid 2x2 PNG fallback if convert is not available
        echo -n -e "\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x02\x00\x00\x00\x02\x08\x02\x00\x00\x00\xfd\xd4\x5c\xdb\x00\x00\x00\x0cIDATx\x9cc`\x00\x00\x00\x02\x00\x01\xe2!\xbc\x33\x00\x00\x00\x00IEND\xaeB`\x82" > "$DEPLOY_ICON"
    fi
fi

# 4️⃣ Ensure linuxdeploy exists
LINUXDEPLOY="${LINUXDEPLOY:-$ROOT/tools/linuxdeploy-x86_64.AppImage}"

if [ ! -f "$LINUXDEPLOY" ]; then
    echo "🔹 Downloading linuxdeploy..."
    mkdir -p "$(dirname "$LINUXDEPLOY")"
    curl -L -o "$LINUXDEPLOY" \
         https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x "$LINUXDEPLOY"
fi

echo "🔹 Using linuxdeploy: $LINUXDEPLOY"
export NO_STRIP=1  # prevents stripping libraries, needed for some environments

# 5️⃣ Run linuxdeploy safely (works in Docker/CI/standalone)
"$LINUXDEPLOY" --appimage-extract-and-run \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/$BIN_NAME" \
    --desktop-file "$APPDIR/$BIN_NAME.desktop" \
    --icon-file "$ICON" \
    --output appimage

# 6️⃣ Move finished AppImage
mkdir -p "$DIST"
mv *.AppImage "$DIST/" 2>/dev/null || true
rm -f "$ICON"

echo "✅ Done! AppImage is in: $DIST"
