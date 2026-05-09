#!/bin/bash
# ═══════════════════════════════════════════════════════════════
#  build_linux.sh  —  One-click build for Linux / Mac
#  Requirements: CMake + g++
#  Ubuntu/Debian: sudo apt install cmake g++
# ═══════════════════════════════════════════════════════════════

echo ""
echo " ===================================================="
echo "  Library System - Linux/Mac Build"
echo " ===================================================="
echo ""

mkdir -p data build
cd build

echo " [1/3] Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo " [ERROR] CMake failed!"
    echo " Install: sudo apt install cmake g++"
    exit 1
fi

echo ""
echo " [2/3] Building..."
make -j4
if [ $? -ne 0 ]; then
    echo " [ERROR] Build failed!"
    exit 1
fi

echo ""
echo " [3/3] Done!"
echo ""
echo " ===================================================="
echo "  Run: ./build/library"
echo " ===================================================="
cd ..
