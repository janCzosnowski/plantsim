#!/usr/bin/env bash
set -e

echo "Building Linux AppImage..."
./scripts/build-linux.sh
echo "Building Windows EXE..."
./scripts/build-windows.sh

echo "All builds complete!"
echo "--- Linux ---"
ls -lh dist/linux || true
echo "--- Windows ---"
ls -lh dist/windows || true