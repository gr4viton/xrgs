#!/bin/bash

# XRGS Linux Launch Script
# Automatically sets up Vulkan SDK environment and runs XRGS

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 XRGS Linux Launcher${NC}"
echo "=================================="

# Configuration
VULKAN_SDK_PATH="/tmp/1.3.275.0/x86_64"
XRGS_EXECUTABLE="./bin_x64/Release/xrgs_app"

# Check if Vulkan SDK exists
if [ ! -d "$VULKAN_SDK_PATH" ]; then
    echo -e "${RED}❌ ERROR: Vulkan SDK not found at $VULKAN_SDK_PATH${NC}"
    echo -e "${YELLOW}💡 Please run the installation steps from README_LINUX.md first${NC}"
    echo "   Specifically: Download and extract Vulkan SDK 1.3.275+"
    exit 1
fi

# Check if XRGS executable exists
if [ ! -f "$XRGS_EXECUTABLE" ]; then
    echo -e "${RED}❌ ERROR: XRGS executable not found at $XRGS_EXECUTABLE${NC}"
    echo -e "${YELLOW}💡 Please build XRGS first:${NC}"
    echo "   cmake -S . -B build -DCMAKE_PREFIX_PATH=\$VULKAN_SDK"
    echo "   cmake --build build --config Release -j\$(nproc)"
    exit 1
fi

# Set up Vulkan SDK environment
echo -e "${BLUE}🔧 Setting up Vulkan SDK environment...${NC}"
export VULKAN_SDK="$VULKAN_SDK_PATH"
export PATH="$VULKAN_SDK/bin:$PATH"
export LD_LIBRARY_PATH="$VULKAN_SDK/lib:$LD_LIBRARY_PATH"
export VK_LAYER_PATH="$VULKAN_SDK/share/vulkan/explicit_layer.d"

# Verify Vulkan setup
echo -e "${BLUE}🔍 Verifying Vulkan installation...${NC}"
if ! command -v vulkaninfo &> /dev/null; then
    echo -e "${RED}❌ ERROR: vulkaninfo not found in PATH${NC}"
    exit 1
fi

# Check GPU and Vulkan support
echo -e "${BLUE}📊 System Information:${NC}"
VULKAN_VERSION=$(vulkaninfo --summary 2>/dev/null | grep "Vulkan Instance Version" | head -1 || echo "Version check failed")
GPU_INFO=$(vulkaninfo --summary 2>/dev/null | grep "GPU id" | head -1 || echo "GPU detection failed")

echo "  Vulkan: $VULKAN_VERSION"
echo "  GPU: $GPU_INFO"

# Check for NVIDIA drivers if NVIDIA GPU detected
if echo "$GPU_INFO" | grep -qi "nvidia"; then
    if command -v nvidia-smi &> /dev/null; then
        NVIDIA_DRIVER=$(nvidia-smi --query-gpu=driver_version --format=csv,noheader,nounits | head -1)
        echo "  NVIDIA Driver: $NVIDIA_DRIVER"
    fi
fi

echo ""

# Launch XRGS
echo -e "${GREEN}🎮 Launching XRGS...${NC}"
echo "Executable: $XRGS_EXECUTABLE"
echo "Working Directory: $(pwd)"
echo ""

# Handle command line arguments
if [ $# -gt 0 ]; then
    echo -e "${BLUE}📝 Command line arguments: $@${NC}"
fi

# Launch the application
echo -e "${GREEN}▶️  Starting XRGS application...${NC}"
exec "$XRGS_EXECUTABLE" "$@" 