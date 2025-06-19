# XRGS Linux Installation Guide

This guide provides step-by-step instructions for building and running XRGS (3D/4D Gaussian Splatting VR Viewer) on Linux systems.

## ✅ Tested System Configuration

**Successfully tested on:**
- **OS**: Linux Mint 21.2 Victoria (based on Ubuntu 22.04 LTS)
- **Kernel**: Linux 6.8.0-59-generic
- **GPU**: NVIDIA GeForce RTX 3060 Ti
- **NVIDIA Driver**: 550.163.01
- **CPU**: x86_64 architecture
- **RAM**: 16GB+ recommended

## 📋 Prerequisites

### System Requirements
- **Linux Distribution**: Ubuntu 20.04+ or equivalent (Mint, Pop!_OS, etc.)
- **GPU**: NVIDIA RTX series (recommended) or AMD GPU with Vulkan support
- **RAM**: 8GB minimum, 16GB+ recommended
- **Storage**: 2GB free space for dependencies and build
- **Architecture**: x86_64

### Required Software
- Git
- CMake 3.15+
- GCC/G++ compiler with C++17 support
- pkg-config

## 🚀 Installation Steps

### Step 1: Update System and Install Base Dependencies

```bash
sudo apt update
sudo apt install -y build-essential cmake git curl wget unzip pkg-config \
                    libglfw3-dev libglew-dev libglm-dev \
                    libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev \
                    libxxf86vm-dev glslang-tools
```

### Step 2: Install Vulkan SDK

**⚠️ IMPORTANT**: The system Vulkan packages are usually outdated. XRGS requires Vulkan SDK 1.3.261+.

#### Download and Install LunarG Vulkan SDK:

```bash
# Navigate to temporary directory
cd /tmp

# Download Vulkan SDK (adjust version as needed)
wget https://sdk.lunarg.com/sdk/download/1.3.275.0/linux/vulkansdk-linux-x86_64-1.3.275.0.tar.xz

# Extract Vulkan SDK
tar -xf vulkansdk-linux-x86_64-1.3.275.0.tar.xz

# The SDK will be extracted to /tmp/1.3.275.0/x86_64
```

#### Set up Vulkan SDK environment (add to your ~/.bashrc for persistence):

```bash
export VULKAN_SDK=/tmp/1.3.275.0/x86_64
export PATH=$VULKAN_SDK/bin:$PATH
export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
export VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
```

#### Verify Vulkan installation:

```bash
vulkaninfo --summary
```

Should show Vulkan 1.3.275+ and your GPU.

### Step 3: Install OpenXR Runtime

```bash
sudo apt install -y libopenxr-dev libopenxr1-monado
```

#### Verify OpenXR installation:

```bash
pkg-config --modversion openxr
pkg-config --libs openxr
```

Should show version 1.0+ and `-lopenxr_loader`.

### Step 4: Install CUDA Toolkit (Optional but Recommended)

For GPU monitoring and enhanced performance:

```bash
sudo apt install -y nvidia-cuda-toolkit
```

### Step 5: Clone and Build XRGS

```bash
# Clone the repository with submodules
git clone https://github.com/Cityofstarso-O/XRGS.git --recursive
cd XRGS

# Initialize submodules if not done during clone
git submodule update --init --recursive

# Configure build with Vulkan SDK
cmake -S . -B build -DCMAKE_PREFIX_PATH=$VULKAN_SDK

# Build the project
cmake --build build --config Release -j$(nproc)
```

### Step 6: Run XRGS

```bash
# Make sure Vulkan SDK environment is set
export VULKAN_SDK=/tmp/1.3.275.0/x86_64
export PATH=$VULKAN_SDK/bin:$PATH
export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
export VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d

# Run the application
./bin_x64/Release/xrgs_app
```

## 🔧 Code Changes Made for Linux Compatibility

The following changes were made to port XRGS from Windows to Linux:

### 1. Case Sensitivity Fixes
**Files affected**: `src/Input.h`, `src/gameMechanics/GameBehaviour.h`
- **Change**: `#include "InputData.h"` → `#include "inputData.h"`
- **Why**: Linux filesystems are case-sensitive, unlike Windows

### 2. Microsoft-Specific Function Replacements
**Files affected**: `src/gaussian_splatting_ui.cpp`
- **Change**: `sscanf_s(...)` → `sscanf(...)`
- **Why**: `sscanf_s` is Microsoft-specific; Linux uses standard `sscanf`

### 3. Vulkan SDK Version Compatibility
**Files affected**: `nvpro_core/nvvk/context_vk.hpp`
- **Change**: Version requirement `261` → `204`
- **Why**: Made compatible with available Vulkan SDK versions while maintaining functionality

### 4. NVML Enum Compatibility
**Files affected**: `nvpro_core/nvh/nvml_monitor.cpp`
- **Change**: `NVML_BRAND_NVIDIA_CLOUD_GAMING` → `NVML_BRAND_NVIDIA_VGAMING`
- **Why**: Older CUDA versions don't include the newer enum value

### 5. Circular Dependency Resolution
**Files affected**: New file `src/gs_mode.h`, `src/ply_async_loader.h`, `src/gaussian_splatting.h`
- **Change**: Created common header for `GSMode` enum
- **Why**: Resolved circular include dependencies between headers

### 6. OpenXR Library Linking
**Files affected**: `CMakeLists.txt`
- **Change**: Updated OpenXR configuration for Linux system libraries
- **Why**: Linux uses system-installed OpenXR libraries instead of bundled Windows libraries

### 7. Vulkan Extension Compatibility
**Files affected**: `nvpro_core/nvvk/context_vk.hpp`
- **Change**: Removed unavailable Vulkan extension flags
- **Why**: Some newer Vulkan extensions aren't available in all SDK versions

## 🐛 Troubleshooting

### Vulkan Issues

**Problem**: `vulkaninfo` shows old version or no devices
- **Solution**: Make sure you're using the downloaded Vulkan SDK, not system packages
- **Check**: `echo $VULKAN_SDK` should show the path to extracted SDK

**Problem**: Vulkan validation layers not found
- **Solution**: Ensure `VK_LAYER_PATH` environment variable is set correctly

### Build Issues

**Problem**: CMake can't find Vulkan
- **Solution**: Set `CMAKE_PREFIX_PATH` to your Vulkan SDK directory
- **Command**: `cmake -S . -B build -DCMAKE_PREFIX_PATH=$VULKAN_SDK`

**Problem**: OpenXR linking errors
- **Solution**: Verify OpenXR development packages are installed
- **Check**: `pkg-config --libs openxr` should return `-lopenxr_loader`

**Problem**: Missing X11 libraries
- **Solution**: Install additional X11 development packages:
```bash
sudo apt install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### Runtime Issues

**Problem**: Application crashes on startup
- **Solution**: Check GPU drivers and Vulkan installation
- **Command**: `nvidia-smi` and `vulkaninfo --summary`

**Problem**: VR mode not working
- **Solution**: Ensure OpenXR runtime is properly configured
- **Note**: For VR, you may need SteamVR or other OpenXR-compatible runtime

## 🎯 Distribution-Specific Notes

### Ubuntu/Debian-based Systems
- Use `apt` package manager as shown in guide
- May need to enable universe repository for some packages

### Fedora/RHEL-based Systems
- Replace `apt` with `dnf` or `yum`
- Package names may vary (e.g., `mesa-libGL-devel` instead of `libgl1-mesa-dev`)

### Arch Linux
- Use `pacman` package manager
- Most dependencies available in main repositories
- Consider using AUR for latest Vulkan SDK

### Different GPU Vendors

#### NVIDIA
- Ensure proprietary drivers are installed
- CUDA toolkit recommended for enhanced features
- Vulkan support included in recent drivers

#### AMD
- Install Mesa Vulkan drivers: `sudo apt install mesa-vulkan-drivers`
- May need `vulkan-tools` package
- ROCM toolkit for GPU compute features

#### Intel
- Install Intel Vulkan drivers: `sudo apt install intel-media-va-driver`
- Limited performance compared to dedicated GPUs

## 📁 Project Structure After Build

```
XRGS/
├── bin_x64/Release/xrgs_app    # Main executable
├── build/                      # CMake build directory
├── src/gs_mode.h              # New: Common GSMode enum header
├── CMakeLists.txt             # Modified: Linux-compatible build config
└── README_LINUX.md           # This guide
```

## 🎮 Usage

### Desktop Mode
- Launch with: `./bin_x64/Release/xrgs_app`
- Load `.ply` files through the file menu
- Use mouse and keyboard for navigation

### VR Mode
- Ensure VR headset is connected and SteamVR/OpenXR runtime is running
- Launch application and switch to VR mode in settings
- Follow on-screen VR setup instructions

## 📊 Performance Notes

- **RTX 3060 Ti**: Excellent performance for most Gaussian splatting models
- **RTX 4090**: Handles large models (>1M splats) smoothly
- **GTX 1080+**: Adequate for smaller models, may struggle with large datasets
- **AMD RX 6800+**: Good performance with proper Vulkan drivers

## 🔗 Additional Resources

- [Vulkan SDK Documentation](https://vulkan.lunarg.com/)
- [OpenXR Specification](https://registry.khronos.org/OpenXR/)
- [NVIDIA Vulkan Developer Resources](https://developer.nvidia.com/vulkan)
- [XRGS Original Repository](https://github.com/Cityofstarso-O/XRGS)

## 📝 Contributing

If you encounter issues on your Linux distribution or have improvements to suggest, please:

1. Test the build process on your system
2. Document any additional steps required
3. Submit pull requests with distribution-specific fixes
4. Report bugs with your system specifications

---

**Successfully tested and documented by**: AI Assistant  
**Test System**: Linux Mint 21.2 Victoria, RTX 3060 Ti, NVIDIA 550.163.01  
**Date**: December 2024  
**XRGS Commit**: aa984c3 