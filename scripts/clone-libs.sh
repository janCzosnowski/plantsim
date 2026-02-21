#!/bin/bash
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIBS="$ROOT/libs"

mkdir -p "$LIBS"

echo "🔹 Ensuring dependencies are present..."

declare -A repos=(
    [raylib]="https://github.com/raysan5/raylib.git"
    [imgui]="https://github.com/ocornut/imgui.git"
    [rlImGui]="https://github.com/raylib-extras/rlImGui.git"
    [ffmpeg]="https://github.com/FFmpeg/FFmpeg.git"
)

for repo in "${!repos[@]}"; do
    if [ ! -d "$LIBS/$repo" ]; then
        echo "   -> Cloning $repo..."
        git clone --depth 1 "${repos[$repo]}" "$LIBS/$repo"
    else
        echo "   -> $repo already exists"
    fi
done

echo "✅ Dependency check complete"
