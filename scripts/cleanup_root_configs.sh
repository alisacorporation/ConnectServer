#!/bin/bash
# Cleanup script to remove duplicate config files from root directory
# These files are now managed in the config/ folder

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== ConnectServer Config Cleanup Script ===${NC}"
echo ""
echo "This script will remove duplicate configuration files from the root directory."
echo "Configuration files are now managed in the config/ folder."
echo ""

# Get script directory and navigate to project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."
cd "$ROOT_DIR"

echo "Current directory: $(pwd)"
echo ""

# Check if files exist
FILES_TO_REMOVE=0
FILES_LIST=()

if [ -f "ConnectServer.ini.example" ]; then
    echo -e "${YELLOW}[FOUND]${NC} ConnectServer.ini.example"
    FILES_LIST+=("ConnectServer.ini.example")
    ((FILES_TO_REMOVE++))
fi

if [ -f "ServerList.dat.example" ]; then
    echo -e "${YELLOW}[FOUND]${NC} ServerList.dat.example"
    FILES_LIST+=("ServerList.dat.example")
    ((FILES_TO_REMOVE++))
fi

if [ $FILES_TO_REMOVE -eq 0 ]; then
    echo -e "${GREEN}No duplicate files found. Root directory is already clean.${NC}"
    echo ""
    exit 0
fi

echo ""
echo "Files to remove: $FILES_TO_REMOVE"
echo ""
echo -e "${YELLOW}WARNING:${NC} This will delete the above files from the root directory."
echo "The same files exist in config/ folder and will be used by the build system."
echo ""
read -p "Do you want to continue? (y/N): " CONFIRM

if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    echo "Operation cancelled."
    exit 0
fi

echo ""
echo "Removing files..."

for file in "${FILES_LIST[@]}"; do
    if rm "$file" 2>/dev/null; then
        echo -e "  ${GREEN}✓${NC} Removed $file"
    else
        echo -e "  ${RED}✗${NC} Failed to remove $file"
    fi
done

echo ""
echo -e "${GREEN}Cleanup completed!${NC}"
echo ""
echo "Configuration files are now managed in config/ folder:"
echo "  - config/ConnectServer.ini.example"
echo "  - config/ServerList.dat.example"
echo ""
echo "The build system will automatically copy these to the output directory."
echo ""
