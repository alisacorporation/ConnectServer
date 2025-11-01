#!/bin/bash
# Build script for ConnectServer (Linux)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== ConnectServer Build Script ===${NC}"

# Parse arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
USE_NCURSES=false
BUILD_TESTS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --ncurses)
            USE_NCURSES=true
            shift
            ;;
        --tests)
            BUILD_TESTS=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug     Build in Debug mode (default: Release)"
            echo "  --clean     Clean build directory before building"
            echo "  --ncurses   Enable ncurses TUI support"
            echo "  --tests     Build unit tests"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check for required tools
command -v cmake >/dev/null 2>&1 || { echo -e "${RED}cmake is required but not installed.${NC}" >&2; exit 1; }
command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || { echo -e "${RED}g++ or clang++ is required but not installed.${NC}" >&2; exit 1; }

# Create build directory
BUILD_DIR="build"
if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${GREEN}Configuring...${NC}"
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$USE_NCURSES" = true ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DUSE_NCURSES=ON"
fi

if [ "$BUILD_TESTS" = true ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DBUILD_TESTS=ON"
fi

cmake .. $CMAKE_ARGS

# Build
echo -e "${GREEN}Building...${NC}"
CPU_COUNT=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build . --config $BUILD_TYPE -j$CPU_COUNT

# Copy config files from config folder to build directory
echo -e "${GREEN}Copying configuration files from config folder...${NC}"
if [ -f "../config/ConnectServer.ini.example" ]; then
    cp ../config/ConnectServer.ini.example ./ConnectServer.ini
    echo -e "  ${GREEN}✓${NC} ConnectServer.ini (from config/ConnectServer.ini.example)"
else
    echo -e "  ${YELLOW}⚠${NC} ConnectServer.ini.example not found in config folder"
fi
if [ -f "../config/ServerList.dat.example" ]; then
    cp ../config/ServerList.dat.example ./ServerList.dat
    echo -e "  ${GREEN}✓${NC} ServerList.dat (from config/ServerList.dat.example)"
else
    echo -e "  ${YELLOW}⚠${NC} ServerList.dat.example not found in config folder"
fi

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "Binary location: ${YELLOW}$BUILD_DIR/ConnectServer${NC}"
echo -e ""
echo -e "To run the server:"
echo -e "  cd $BUILD_DIR"
echo -e "  ./ConnectServer"
