# XRGS
This a 3dgs/4dgs viewer that supports easy switching between PC and headset. The project is based on [vk_gaussian_splatting](https://github.com/nvpro-samples/vk_gaussian_splatting.git) and has added support for 4DGS(especially [Spacetime Gaussian Lite version](https://oppo-us-research.github.io/SpacetimeGaussians-website/)), designed specifically for viewing on PC and streaming for PCVR. BTW Khronos are you real human?
## Requirements
- **Vulkan >= 1.3 SDK** from LunarG – [Download](https://vulkan.lunarg.com/).  
- **OpenXR >= 1.0.34 SDK** from Khronos – [Download](https://github.com/KhronosGroup/OpenXR-SDK/releases/download/release-1.0.34/OpenXR.Loader.1.0.34.nupkg).  
- **CUDA >= v11.7** is **optional** and can be used to activate **NVML GPU monitoring** in the sample – [Download](https://developer.nvidia.com/cuda-downloads).

After downloading, make sure you have environment variables related to Vulkan SDK(and CUDA Toolkit) on your operate system, and **unzip OpenXR SDK** to `./3rdparty/OpenXR.Loader.1.x.xx`.
## Build and Run
``` sh
git clone https://github.com/Cityofstarso-O/XRGS.git --recursive
cd XRGS

cmake -S . -B build
cmake --build build --config Release
```
After building the project, direct to `./bin_x64/Release/` where you can find `XRGS.exe`.
## View your own .ply file
The viewer has two gaussian mode where you can switch at bottom right of the window:
- **3dgs**:
  - This mode supports .ply file following the format from the original [3DGS](https://github.com/graphdeco-inria/gaussian-splatting.git) paper.
- **spacetime-lite**:
  - This mode supports .ply file following the **lite** format from the original [Spacetime Gaussian Feature Splatting for Real-Time Dynamic View Synthesis](https://oppo-us-research.github.io/SpacetimeGaussians-website/) paper.
## TODO
- Includes the functionality to move using a controller.
  - For now, you can only move and spin your headset to move in the scene.
- Fix the bug that image presented on PC is dark when using PCVR.
  - This is due to some headsets do not support UNORM image format. To gain the right result in headset, we need to do inverse Gamma Correction manually. Then the blitting image on PC is darker following that.
- Optimize performance.
  - considering using vulkan multi view extension to decrease times of draw call and allocate only one swapchain.
## License
This project follows the original License from [vk_gaussian_splatting](https://github.com/nvpro-samples/vk_gaussian_splatting.git).