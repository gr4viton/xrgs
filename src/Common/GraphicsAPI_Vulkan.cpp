// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <GraphicsAPI_Vulkan.h>

#if defined(XR_USE_GRAPHICS_API_VULKAN)

#define VULKAN_CHECK(x, y)                                                                         \
    {                                                                                              \
        VkResult result = (x);                                                                     \
        if (result != VK_SUCCESS) {                                                                \
            std::cout << "ERROR: VULKAN: " << std::hex << "0x" << result << std::dec << std::endl; \
            std::cout << "ERROR: VULKAN: " << y << std::endl;                                      \
        }                                                                                          \
    }

#if defined(__ANDROID__) && !defined(VK_API_MAKE_VERSION)
#define VK_MAKE_API_VERSION(variant, major, minor, patch) VK_MAKE_VERSION(major, minor, patch)
#endif

static bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkMemoryPropertyFlags requirementsMask, uint32_t *typeIndex) {
    // Search memory types to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
};

static VkFormat ToVkFormat(GraphicsAPI::VertexType type) {
    switch (type) {
    case GraphicsAPI::VertexType::FLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case GraphicsAPI::VertexType::VEC2:
        return VK_FORMAT_R32G32_SFLOAT;
    case GraphicsAPI::VertexType::VEC3:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case GraphicsAPI::VertexType::VEC4:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GraphicsAPI::VertexType::INT:
        return VK_FORMAT_R32_SINT;
    case GraphicsAPI::VertexType::IVEC2:
        return VK_FORMAT_R32G32_SINT;
    case GraphicsAPI::VertexType::IVEC3:
        return VK_FORMAT_R32G32B32_SINT;
    case GraphicsAPI::VertexType::IVEC4:
        return VK_FORMAT_R32G32B32A32_SINT;
    case GraphicsAPI::VertexType::UINT:
        return VK_FORMAT_R32_UINT;
    case GraphicsAPI::VertexType::UVEC2:
        return VK_FORMAT_R32G32_UINT;
    case GraphicsAPI::VertexType::UVEC3:
        return VK_FORMAT_R32G32B32_UINT;
    case GraphicsAPI::VertexType::UVEC4:
        return VK_FORMAT_R32G32B32A32_UINT;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}
VkDescriptorType ToVkDescrtiptorType(const GraphicsAPI::DescriptorInfo &descInfo) {
    VkDescriptorType vkType;
    switch (descInfo.type) {
    default:
    case GraphicsAPI::DescriptorInfo::Type::BUFFER: {
        vkType = descInfo.readWrite ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    }
    case GraphicsAPI::DescriptorInfo::Type::IMAGE: {
        vkType = descInfo.readWrite ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    }
    case GraphicsAPI::DescriptorInfo::Type::SAMPLER: {
        vkType = VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
    }
    }
    return vkType;
}

void GraphicsAPI_Vulkan::init() {
    // Vulkan creation context information (see nvvk::Context)
    static VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR baryFeaturesKHR = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR};
    static VkPhysicalDeviceMeshShaderFeaturesEXT meshFeaturesEXT = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
    nvvk::ContextCreateInfo vkSetup;
    vkSetup.setVersion(1, 3);
    vkSetup.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    vkSetup.addDeviceExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME, false, &meshFeaturesEXT);
    vkSetup.addDeviceExtension(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, false, &baryFeaturesKHR);
    vkSetup.addDeviceExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);  // for ImGui
    vkSetup.addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    nvvkhl::addSurfaceExtensions(vkSetup.instanceExtensions);

    // from meshlettest.cpp sample
    vkSetup.fnDisableFeatures = [](VkStructureType sType, void* pFeatureStruct) {
      if(sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT)
      {
        auto* feature = reinterpret_cast<VkPhysicalDeviceMeshShaderFeaturesEXT*>(pFeatureStruct);
        // enabling and not using it may cost a tiny bit of performance on NV hardware
        feature->meshShaderQueries = VK_FALSE;
        // disable for the time beeing
        feature->primitiveFragmentShadingRateMeshShader = VK_FALSE;
      }
    };

    vkContext = std::make_unique<nvvk::Context>();
    vkContext->init(vkSetup);

    instance         = vkContext->m_instance;
    physicalDevice   = vkContext->m_physicalDevice;
    device           = vkContext->m_device;
    queueFamilyIndex = vkContext->m_queueGCT.familyIndex;
    queueIndex       = vkContext->m_queueGCT.queueIndex;
    queue            = vkContext->m_queueGCT.queue;
}

XrResult GraphicsAPI_Vulkan::init(XrInstance m_xrInstance, XrSystemId systemId)
{
    // Instance
    LoadPFN_XrFunctions(m_xrInstance);

    XrGraphicsRequirementsVulkanKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR};
    OPENXR_LOG(xrGetVulkanGraphicsRequirementsKHR(m_xrInstance, systemId, &graphicsRequirements), "Failed to get Graphics Requirements for Vulkan.");

    VkApplicationInfo ai;
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pNext = nullptr;
    ai.pApplicationName = "OpenXR Tutorial - Vulkan";
    ai.applicationVersion = 1;
    ai.pEngineName = "OpenXR Tutorial - Vulkan Engine";
    ai.engineVersion = 1;
    ai.apiVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported), XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported), 0);

    uint32_t instanceExtensionCount = 0;
    VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr), "Failed to enumerate InstanceExtensionProperties.");

    std::vector<VkExtensionProperties> instanceExtensionProperties;
    instanceExtensionProperties.resize(instanceExtensionCount);
    VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensionProperties.data()), "Failed to enumerate InstanceExtensionProperties.");
    const std::vector<std::string> &openXrInstanceExtensionNames = GetInstanceExtensionsForOpenXR(m_xrInstance, systemId);
    for (const std::string &requestExtension : openXrInstanceExtensionNames) {
        for (const VkExtensionProperties &extensionProperty : instanceExtensionProperties) {
            if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
                continue;
            else
                activeInstanceExtensions.push_back(requestExtension.c_str());
            break;
        }
    }

    activeInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    activeInstanceExtensions.push_back("VK_KHR_surface");
    activeInstanceExtensions.push_back("VK_KHR_win32_surface");
    activeInstanceExtensions.push_back("VK_KHR_get_surface_capabilities2");

    VkInstanceCreateInfo instanceCI;
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCI.pNext = nullptr;
    instanceCI.flags = 0;
    instanceCI.pApplicationInfo = &ai;
    instanceCI.enabledLayerCount = static_cast<uint32_t>(activeInstanceLayers.size());
    instanceCI.ppEnabledLayerNames = activeInstanceLayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(activeInstanceExtensions.size());
    instanceCI.ppEnabledExtensionNames = activeInstanceExtensions.data();
    VULKAN_CHECK(vkCreateInstance(&instanceCI, nullptr, &instance), "Failed to create Vulkan Instance.");

    // Physical Device
    uint32_t physicalDeviceCount = 0;
    std::vector<VkPhysicalDevice> physicalDevices;
    VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr), "Failed to enumerate PhysicalDevices.");
    physicalDevices.resize(physicalDeviceCount);
    VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()), "Failed to enumerate PhysicalDevices.");

    VkPhysicalDevice physicalDeviceFromXR;
    OPENXR_LOG(xrGetVulkanGraphicsDeviceKHR(m_xrInstance, systemId, instance, &physicalDeviceFromXR),
               "Failed to get Graphics Device for Vulkan.");
    auto physicalDeviceFromXR_it = std::find(physicalDevices.begin(), physicalDevices.end(), physicalDeviceFromXR);
    if (physicalDeviceFromXR_it != physicalDevices.end()) {
        physicalDevice = *physicalDeviceFromXR_it;
    } else {
        std::cout << "ERROR: Vulkan: Failed to find PhysicalDevice for OpenXR." << std::endl;
        // Select the first available device.
        physicalDevice = physicalDevices[0];
    }

    // Device
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
    queueFamilyProperties.resize(queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs;
    std::vector<std::vector<float>> queuePriorities;
    queuePriorities.resize(queueFamilyProperties.size());
    deviceQueueCIs.resize(queueFamilyProperties.size());
    for (size_t i = 0; i < deviceQueueCIs.size(); i++) {
        for (size_t j = 0; j < queueFamilyProperties[i].queueCount; j++)
            queuePriorities[i].push_back(1.0f);

        deviceQueueCIs[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCIs[i].pNext = nullptr;
        deviceQueueCIs[i].flags = 0;
        deviceQueueCIs[i].queueFamilyIndex = static_cast<uint32_t>(i);
        deviceQueueCIs[i].queueCount = queueFamilyProperties[i].queueCount;
        deviceQueueCIs[i].pQueuePriorities = queuePriorities[i].data();

        if (BitwiseCheck(queueFamilyProperties[i].queueFlags, VkQueueFlags(VK_QUEUE_GRAPHICS_BIT)) && queueFamilyIndex == 0xFFFFFFFF && queueIndex == 0xFFFFFFFF) {
            queueFamilyIndex = static_cast<uint32_t>(i);
            queueIndex = 0;
        }
    }

    uint32_t deviceExtensionCount = 0;
    VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, 0), "Failed to enumerate DeviceExtensionProperties.");
    std::vector<VkExtensionProperties> deviceExtensionProperties;
    deviceExtensionProperties.resize(deviceExtensionCount);

    VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, deviceExtensionProperties.data()), "Failed to enumerate DeviceExtensionProperties.");
    const std::vector<std::string> &openXrDeviceExtensionNames = GetDeviceExtensionsForOpenXR(m_xrInstance, systemId);
    for (const std::string &requestExtension : openXrDeviceExtensionNames) {
        for (const VkExtensionProperties &extensionProperty : deviceExtensionProperties) {
            if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
                continue;
            else
                activeDeviceExtensions.push_back(requestExtension.c_str());
            break;
        }
    }
    activeDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    activeDeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    activeDeviceExtensions.push_back(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
    activeDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR baryFeaturesKHR = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR};
    baryFeaturesKHR.fragmentShaderBarycentric = true;
    VkPhysicalDeviceMeshShaderFeaturesEXT meshFeaturesEXT = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
    meshFeaturesEXT.meshShader = true;
    baryFeaturesKHR.pNext      = &meshFeaturesEXT;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    VkDeviceCreateInfo deviceCI;
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.pNext                   = &baryFeaturesKHR;
    deviceCI.flags = 0;
    deviceCI.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCIs.size());
    deviceCI.pQueueCreateInfos = deviceQueueCIs.data();
    deviceCI.enabledLayerCount = 0;
    deviceCI.ppEnabledLayerNames = nullptr;
    deviceCI.enabledExtensionCount = static_cast<uint32_t>(activeDeviceExtensions.size());
    deviceCI.ppEnabledExtensionNames = activeDeviceExtensions.data();
    deviceCI.pEnabledFeatures = &features;
    VULKAN_CHECK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device), "Failed to create Device.");

    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
}

GraphicsAPI_Vulkan::~GraphicsAPI_Vulkan() {
    if(vkContext)
    {
        vkContext->deinit();
        vkContext.reset();
        return;
    }
    // below objects might be nullptr due to failing to be created when request OpenXR env
    if(device)
    {
        vkDeviceWaitIdle(device);
        vkDestroyDevice(device, nullptr);
    }
    if(instance)
    {
        vkDestroyInstance(instance, nullptr); 
    }
}

void *GraphicsAPI_Vulkan::GetGraphicsBinding() {
    graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};
    graphicsBinding.instance = instance;
    graphicsBinding.physicalDevice = physicalDevice;
    graphicsBinding.device = device;
    graphicsBinding.queueFamilyIndex = queueFamilyIndex;
    graphicsBinding.queueIndex = queueIndex;
    return &graphicsBinding;
}

XrSwapchainImageBaseHeader *GraphicsAPI_Vulkan::AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) {
    swapchainImagesMap[swapchain].first = type;
    swapchainImagesMap[swapchain].second.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});
    return reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImagesMap[swapchain].second.data());
}

void *GraphicsAPI_Vulkan::CreateImage(const ImageCreateInfo &imageCI) {
    VkImage image{};
    VkImageCreateInfo vkImageCI;
    vkImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCI.pNext = nullptr;
    vkImageCI.flags = (imageCI.cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0) | (imageCI.dimension == 3 ? VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT : 0);
    vkImageCI.imageType = VkImageType(imageCI.dimension - 1);
    vkImageCI.format = (VkFormat)imageCI.format;
    vkImageCI.extent = {imageCI.width, imageCI.height, imageCI.depth};
    vkImageCI.mipLevels = imageCI.mipLevels;
    vkImageCI.arrayLayers = imageCI.arrayLayers;
    vkImageCI.samples = VkSampleCountFlagBits(imageCI.sampleCount);
    vkImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | (imageCI.colorAttachment ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0) | (imageCI.depthAttachment ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0);
    vkImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCI.queueFamilyIndexCount = 0;
    vkImageCI.pQueueFamilyIndices = nullptr;
    vkImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VULKAN_CHECK(vkCreateImage(device, &vkImageCI, nullptr, &image), "Failed to create Image");

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    VkDeviceMemory memory{};
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = memoryRequirements.size;

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
    MemoryTypeFromProperties(physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocateInfo.memoryTypeIndex);

    VULKAN_CHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &memory), "Failed to allocate Memory.");
    VULKAN_CHECK(vkBindImageMemory(device, image, memory, 0), "Failed to bind Memory to Image.");

    imageResources[image] = {memory, imageCI};
    imageStates[image] = vkImageCI.initialLayout;

    return (void *)image;
}

void GraphicsAPI_Vulkan::DestroyImage(void *&image) {
    VkImage vkImage = (VkImage)image;
    VkDeviceMemory memory = imageResources[vkImage].first;
    vkFreeMemory(device, memory, nullptr);
    vkDestroyImage(device, vkImage, nullptr);
    imageResources.erase(vkImage);
    imageStates.erase(vkImage);
    image = nullptr;
}

void *GraphicsAPI_Vulkan::CreateImageView(const ImageViewCreateInfo &imageViewCI) {
    VkImageView imageView{};
    VkImageViewCreateInfo vkImageViewCI;
    vkImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCI.pNext = nullptr;
    vkImageViewCI.flags = 0;
    vkImageViewCI.image = (VkImage)imageViewCI.image;
    vkImageViewCI.viewType = VkImageViewType(imageViewCI.view);
    vkImageViewCI.format = (VkFormat)imageViewCI.format;
    vkImageViewCI.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    vkImageViewCI.subresourceRange.aspectMask = VkImageAspectFlagBits(imageViewCI.aspect);
    vkImageViewCI.subresourceRange.baseMipLevel = imageViewCI.baseMipLevel;
    vkImageViewCI.subresourceRange.levelCount = imageViewCI.levelCount;
    vkImageViewCI.subresourceRange.baseArrayLayer = imageViewCI.baseArrayLayer;
    vkImageViewCI.subresourceRange.layerCount = imageViewCI.layerCount;
    VULKAN_CHECK(vkCreateImageView(device, &vkImageViewCI, nullptr, &imageView), "Failed to create ImageView.");

    imageViewResources[imageView] = imageViewCI;
    return (void *)imageView;
}

void GraphicsAPI_Vulkan::DestroyImageView(void *&imageView) {
    VkImageView vkImageView = (VkImageView)imageView;
    vkDestroyImageView(device, vkImageView, nullptr);
    imageViewResources.erase(vkImageView);
    imageView = nullptr;
}

void *GraphicsAPI_Vulkan::CreateSampler(const SamplerCreateInfo &samplerCI) {
    VkSampler sampler{};
    VkSamplerCreateInfo vkSamplerCI;
    vkSamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCI.pNext = nullptr;
    vkSamplerCI.flags = 0;
    vkSamplerCI.magFilter = static_cast<VkFilter>(samplerCI.magFilter);
    vkSamplerCI.minFilter = static_cast<VkFilter>(samplerCI.minFilter);
    vkSamplerCI.mipmapMode = static_cast<VkSamplerMipmapMode>(samplerCI.mipmapMode);
    vkSamplerCI.addressModeU = static_cast<VkSamplerAddressMode>(samplerCI.addressModeS);
    vkSamplerCI.addressModeV = static_cast<VkSamplerAddressMode>(samplerCI.addressModeT);
    vkSamplerCI.addressModeW = static_cast<VkSamplerAddressMode>(samplerCI.addressModeR);
    vkSamplerCI.mipLodBias = samplerCI.mipLodBias;
    vkSamplerCI.anisotropyEnable = false;
    vkSamplerCI.maxAnisotropy = 0.0f;
    vkSamplerCI.compareEnable = samplerCI.compareEnable;
    vkSamplerCI.compareOp = static_cast<VkCompareOp>(samplerCI.compareOp);
    vkSamplerCI.minLod = samplerCI.minLod;
    vkSamplerCI.maxLod = samplerCI.maxLod;
    bool opaque = samplerCI.borderColor[3] == 1.0f;
    bool white = samplerCI.borderColor[0] == 1.0f && samplerCI.borderColor[1] == 1.0f && samplerCI.borderColor[2] == 1.0f && opaque;
    vkSamplerCI.borderColor = static_cast<VkBorderColor>((white ? 4 : opaque ? 2
                                                                             : 0));
    vkSamplerCI.unnormalizedCoordinates = false;

    VULKAN_CHECK(vkCreateSampler(device, &vkSamplerCI, nullptr, &sampler), "Failed to create Sampler.");
    return (void *)sampler;
}

void GraphicsAPI_Vulkan::DestroySampler(void *&sampler) {
    vkDestroySampler(device, (VkSampler)sampler, nullptr);
    sampler = nullptr;
}

void GraphicsAPI_Vulkan::LoadPFN_XrFunctions(XrInstance m_xrInstance) {
    OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction *)&xrGetVulkanGraphicsRequirementsKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction *)&xrGetVulkanInstanceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction *)&xrGetVulkanDeviceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction *)&xrGetVulkanGraphicsDeviceKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
}

std::vector<std::string> GraphicsAPI_Vulkan::GetInstanceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId) {
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(m_xrInstance, systemId, 0, &extensionNamesSize, nullptr), "Failed to get Vulkan Instance Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(m_xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()), "Failed to get Vulkan Instance Extensions.");

    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' ')) {
        extensions.push_back(extension);
    }
    return extensions;
}

std::vector<std::string> GraphicsAPI_Vulkan::GetDeviceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId) {
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(m_xrInstance, systemId, 0, &extensionNamesSize, nullptr), "Failed to get Vulkan Device Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(m_xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()), "Failed to get Vulkan Device Extensions.");

    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' ')) {
        extensions.push_back(extension);
    }
    return extensions;
}

const std::vector<int64_t> GraphicsAPI_Vulkan::GetSupportedColorSwapchainFormats() {
    return {
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM};
}
const std::vector<int64_t> GraphicsAPI_Vulkan::GetSupportedDepthSwapchainFormats() {
    return {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D16_UNORM};
}
#endif
