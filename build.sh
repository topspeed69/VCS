#!/bin/bash
set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building VCS Project...${NC}"

# Install dependencies if missing
echo -e "${YELLOW}Checking and installing dependencies...${NC}"
sudo apt-get update
sudo apt-get install -y cmake g++ libssl-dev nlohmann-json3-dev

# Clean build directory
echo -e "${YELLOW}Cleaning build directory...${NC}"
rm -rf build
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake ..

# Build
echo -e "${YELLOW}Building project...${NC}"
cmake --build .

# Copy executable to root directory
echo -e "${YELLOW}Copying executable...${NC}"
cp vcs ..

cd ..

echo -e "${GREEN}Build complete! You can now use './vcs' from this directory.${NC}"