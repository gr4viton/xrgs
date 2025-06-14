/*
 * Copyright (c) 2023-2025, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025, NVIDIA CORPORATION.
 * SPDX-License-Identifier: Apache-2.0
 */
#define SWITCH_CHECK(x, y)                                                                                               \
  {                                                                                                                    \
    XrResult result = (x);                                                                                             \
    if(!XR_SUCCEEDED(result))                                                                                          \
    {                                                                                                                  \
      std::cerr << "ERROR: Fail to switch mode: " << y <<  std::endl;                                                             \
      ifSwitchMode = true;                                                                                     \
      return;                                                                                                   \
    }                                                                                                                  \
  } 

#include <gaussian_splatting.h>
#include <Common/GraphicsAPI.h>
#include <Common/GraphicsAPI_Vulkan.h>
#include "openxr_env.h"

class AppCtrl
{
public:
  std::unique_ptr<nvvkhl::Application> app = nullptr;
  std::shared_ptr<GraphicsAPI_Vulkan> graphicsAPI = nullptr;

  bool ifSwitchMode = false;
  Mode currentMode  = Mode::PC;

  GaussianSplatting::RenderSettings m_renderSettings;

  void run()
  {
    while(true)
    {
      ifSwitchMode = false;
      if(currentMode == Mode::PC)
      {
        runPC();
      }
      else if(currentMode == Mode::XR)
      {
        runXR();
      }
      if(ifSwitchMode)
      {
        currentMode = currentMode == Mode::XR ? Mode::PC : Mode::XR;
      }
      else
      {
        break;
      }
    }
  }

  void runPC() {
    // Vulkan Context
    graphicsAPI = std::make_shared<GraphicsAPI_Vulkan>();
    graphicsAPI->init();
    // Application setup
    nvvkhl::ApplicationCreateInfo appSetup;
    appSetup.name                  = fmt::format("{}", PROJECT_NAME);
    appSetup.vSync                 = true;
    appSetup.hasUndockableViewport = true;
    appSetup.instance              = graphicsAPI->instance;
    appSetup.device                = graphicsAPI->device;
    appSetup.physicalDevice        = graphicsAPI->physicalDevice;
    appSetup.queues.push_back({graphicsAPI->queueFamilyIndex, graphicsAPI->queueIndex, graphicsAPI->queue});

    // Setting up the layout of the application
    appSetup.dockSetup = [](ImGuiID viewportID) {
      // right side panel container
      ImGuiID settingID = ImGui::DockBuilderSplitNode(viewportID, ImGuiDir_Right, 0.25F, nullptr, &viewportID);
      ImGui::DockBuilderDockWindow("Settings", settingID);
      ImGui::DockBuilderDockWindow("Misc", settingID);

      // bottom panel container
      ImGuiID memoryID = ImGui::DockBuilderSplitNode(viewportID, ImGuiDir_Down, 0.35F, nullptr, &viewportID);
      ImGui::DockBuilderDockWindow("Memory Statistics", memoryID);
      ImGuiID profilerID = ImGui::DockBuilderSplitNode(memoryID, ImGuiDir_Right, 0.33F, nullptr, &memoryID);
      ImGui::DockBuilderDockWindow("Profiler", profilerID);
    };

    // Create the application
    auto app = std::make_unique<nvvkhl::Application>(appSetup);

    // create the profiler element
    auto profiler = std::make_shared<nvvkhl::ElementProfiler>(true);
    // create the core of the sample
    auto gaussianSplatting = std::make_shared<GaussianSplatting>(profiler, nullptr);

    // Add all application elements including our sample specific gaussianSplatting
    app->addElement(gaussianSplatting);
    app->addElement(std::make_shared<nvvkhl::ElementCamera>());
    app->addElement(std::make_shared<nvvkhl::ElementDefaultWindowTitle>("", fmt::format("({})", "GLSL")));  // Window title info//
    app->addElement(profiler);
    app->addElement(std::make_shared<nvvkhl::ElementNvml>());
    //
    gaussianSplatting->registerRecentFilesHandler();
    gaussianSplatting->m_mode = Mode::PC;
    app->run();
    ifSwitchMode = app->ifSwitchMode();
    gaussianSplatting->getRenderSettings(m_renderSettings);
    app.reset();
    gaussianSplatting.reset();  // this module used vmaAllocator which is corresponded with vk objects, so deconstruct before graphicsAPI
    profiler.reset();
    graphicsAPI.reset();
  }

  void runXR() {
    // Openxr-Vulkan Context
    std::shared_ptr<OpenXREnv> xrEnv = std::make_shared<OpenXREnv>(GraphicsAPI_Type::VULKAN);
    graphicsAPI                      = std::make_shared<GraphicsAPI_Vulkan>();
    SWITCH_CHECK(xrEnv->InitVulkan(), "fail to create openxr instance or get systemId");
    SWITCH_CHECK(graphicsAPI->init(xrEnv->GetXrInstance(), xrEnv->GetXrSystemId()), "fail to create openxr compatible vulkan objects");
    xrEnv->GetGraphicsAPI(graphicsAPI);
    SWITCH_CHECK(xrEnv->InitSession(), "fail to create openxr session or swapchain");
    // Application setup
    nvvkhl::ApplicationCreateInfo appSetup;
    appSetup.name                  = fmt::format("{}", PROJECT_NAME);
    appSetup.vSync                 = true;
    appSetup.hasUndockableViewport = true;
    appSetup.instance              = graphicsAPI->instance;
    appSetup.device                = graphicsAPI->device;
    appSetup.physicalDevice        = graphicsAPI->physicalDevice;
    appSetup.queues.push_back({graphicsAPI->queueFamilyIndex, graphicsAPI->queueIndex, graphicsAPI->queue});

    // Setting up the layout of the application
    appSetup.dockSetup = [](ImGuiID viewportID) {
      // right side panel container
      ImGuiID settingID = ImGui::DockBuilderSplitNode(viewportID, ImGuiDir_Right, 0.25F, nullptr, &viewportID);
      ImGui::DockBuilderDockWindow("Settings", settingID);
      ImGui::DockBuilderDockWindow("Misc", settingID);

      // bottom panel container
      ImGuiID memoryID = ImGui::DockBuilderSplitNode(viewportID, ImGuiDir_Down, 0.35F, nullptr, &viewportID);
      ImGui::DockBuilderDockWindow("Memory Statistics", memoryID);
      ImGuiID profilerID = ImGui::DockBuilderSplitNode(memoryID, ImGuiDir_Right, 0.33F, nullptr, &memoryID);
      ImGui::DockBuilderDockWindow("Profiler", profilerID);
    };

    // Create the application
    auto app = std::make_unique<nvvkhl::Application>(appSetup);

    // create the profiler element
    auto profiler = std::make_shared<nvvkhl::ElementProfiler>(true);
    // create the core of the sample
    auto gaussianSplatting = std::make_shared<GaussianSplatting>(profiler, nullptr);
    gaussianSplatting->m_headsetSupportUnorm = xrEnv->SupportUnorm();
    // Add all application elements including our sample specific gaussianSplatting
    app->addElement(gaussianSplatting); // this should be the first to add
    app->addElement(std::make_shared<nvvkhl::ElementCamera>());
    app->addElement(std::make_shared<nvvkhl::ElementDefaultWindowTitle>("", fmt::format("({})", "GLSL")));  // Window title info//
    app->addElement(profiler);
    app->addElement(std::make_shared<nvvkhl::ElementNvml>());
    //
    gaussianSplatting->registerRecentFilesHandler();
    gaussianSplatting->setRenderSettings(m_renderSettings);
    gaussianSplatting->m_mode = Mode::XR;

    std::function<void(nvvkhl::Application*)> renderFunc = [xrEnv, gaussianSplatting](nvvkhl::Application* app) {
      xrEnv->PollEvents();
      if(!xrEnv->AppRunning())
      {
        app->close();
      }
      VkCommandBuffer cmd = app->beginFrame();
      if(cmd != nullptr && xrEnv->SessionRunning())
      {
          RenderLayerInfo renderLayerInfo;
          if(xrEnv->BeginFrame(renderLayerInfo))
          {
            if(xrEnv->RenderLayer(renderLayerInfo, cmd, gaussianSplatting))
            {
              renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.layerProjection));
            }
          }
          app->drawFrame(cmd);
          app->endFrame(cmd);   // submit before release swapchain image
          app->presentFrame();
          xrEnv->EndFrame(renderLayerInfo);
      }
    };

    app->runXR(renderFunc);
    ifSwitchMode = app->ifSwitchMode();
    app.reset();
    renderFunc = nullptr;
    gaussianSplatting.reset();  // this module used vmaAllocator which is corresponded with vk objects, so deconstruct before graphicsAPI
    profiler.reset();
    xrEnv.reset();
    graphicsAPI.reset();
  }
};

// create, setup and run an nvvkhl::Application
// with a GaussianSplatting element.
int main(int argc, char** argv)
{
  std::unique_ptr<AppCtrl> appController = std::make_unique<AppCtrl>();
  appController->run();
  

  return 0;
}
