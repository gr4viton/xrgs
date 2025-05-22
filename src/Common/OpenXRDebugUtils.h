// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#pragma once
#include <vulkan/vulkan.h>
#include <HelperFunctions.h>
#include <OpenXRHelper.h>
#define VULKAN_CHECK(x, y)                                                                                             \
  {                                                                                                                    \
    VkResult result = (x);                                                                                             \
    if(result != VK_SUCCESS)                                                                                           \
    {                                                                                                                  \
      std::cout << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl;                           \
      std::cout << "ERROR: VULKAN: " << y << std::endl;                                                                \
    }                                                                                                                  \
  }

XrBool32 OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

XrDebugUtilsMessengerEXT CreateOpenXRDebugUtilsMessenger(XrInstance m_xrInstance);
void DestroyOpenXRDebugUtilsMessenger(XrInstance m_xrInstance, XrDebugUtilsMessengerEXT debugUtilsMessenger);
