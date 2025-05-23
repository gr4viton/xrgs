#include <DebugOutput.h>
#include <GraphicsAPI_Vulkan.h>
#include <OpenXRDebugUtils.h>

#include <xr_linear_algebra.h>
#include "openxr_env.h"
#include <Common/GraphicsAPI.h>
#include <OpenXRHelper.h>
#include "gaussian_splatting.h"

XrVector3f operator-(XrVector3f a, XrVector3f b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
XrVector3f operator*(XrVector3f a, float b) {
    return {a.x * b, a.y * b, a.z * b};
}

OpenXREnv::OpenXREnv(GraphicsAPI_Type apiType)
    : m_apiType(apiType)
{
    // Check API compatibility with Platform.
    if (!CheckGraphicsAPI_TypeIsValidForPlatform(m_apiType)) {
        XR_TUT_LOG_ERROR("ERROR: The provided Graphics API is not valid for this platform.");
        DEBUG_BREAK;
    }
}

OpenXREnv::~OpenXREnv()
{
    std::cout << "Destroying OpenXR\n";
    Destroy();
    m_graphicsAPI = nullptr;
}

bool OpenXREnv::SessionRunning() const {
    return m_sessionRunning;
}

bool OpenXREnv::AppRunning()const 
{
    return m_applicationRunning;
}

XrResult OpenXREnv::InitVulkan()
{
    std::string errString = "Failed to initialize OpenXR";

    std::cout << "Begin initializing OpenXR\n";
    OPENXR_LOG(CreateInstance(), errString);
    CreateDebugMessenger();
    OPENXR_LOG(GetInstanceProperties(), errString);
    OPENXR_LOG(GetSystemID(), errString);
    return XR_SUCCESS;
}
XrResult OpenXREnv::InitSession()
{
    std::string errString = "Failed to initialize OpenXR";

    std::cout << "Begin initializing session\n";
    OPENXR_LOG(GetViewConfigurationViews(), errString)
    OPENXR_LOG(GetEnvironmentBlendModes(), errString)

    OPENXR_LOG(CreateSession(), errString);
    OPENXR_LOG(CreateReferenceSpace(), errString);
    OPENXR_LOG(CreateSwapchains(), errString);
    return XR_SUCCESS;
}
void OpenXREnv::Destroy() {
    DestroySwapchains();
    DestroyReferenceSpace();
    DestroySession();
    DestroyDebugMessenger();
    DestroyInstance();
}

XrResult OpenXREnv::CreateInstance()
{
    // Fill out an XrApplicationInfo structure detailing the names and OpenXR version.
    // The application/engine name and version are user-definied. These may help IHVs or runtimes.
    XrApplicationInfo AI;
    strncpy(AI.applicationName, "XRGS Viewer", XR_MAX_APPLICATION_NAME_SIZE);
    AI.applicationVersion = 1;
    strncpy(AI.engineName, "XRGS Engine", XR_MAX_ENGINE_NAME_SIZE);
    AI.engineVersion = 1;
    AI.apiVersion = XR_CURRENT_API_VERSION;
    // Add additional instance layers/extensions that the application wants.
    // Add both required and requested instance extensions.
    {
        m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
        // Ensure m_apiType is already defined when we call this line.
        m_instanceExtensions.push_back(GetGraphicsAPIInstanceExtensionString(m_apiType));
    }
    // Get all the API Layers from the OpenXR runtime.
    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    OPENXR_LOG(xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr), "Failed to enumerate ApiLayerProperties.");
    apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    OPENXR_LOG(xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()),
               "Failed to enumerate ApiLayerProperties.");
    // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
    for (auto &requestLayer : m_apiLayers) {
        for (auto &layerProperty : apiLayerProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0) {
                continue;
            } else {
                m_activeAPILayers.push_back(requestLayer.c_str());
                break;
            }
        }
    }
    // Get all the Instance Extensions from the OpenXR instance.
    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> extensionProperties;
    OPENXR_LOG(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr),
               "Failed to enumerate InstanceExtensionProperties.");
    extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    OPENXR_LOG(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()),
               "Failed to enumerate InstanceExtensionProperties.");
    // Check the requested Instance Extensions against the ones from the OpenXR runtime.
    // If an extension is found add it to Active Instance Extensions.
    // Log error if the Instance Extension is not found.
    for (auto &requestedInstanceExtension : m_instanceExtensions) {
        bool found = false;
        for (auto &extensionProperty : extensionProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0) {
                continue;
            } else {
                m_activeInstanceExtensions.push_back(requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found) {
            XR_TUT_LOG_ERROR("Failed to find OpenXR instance extension: " << requestedInstanceExtension);
        }
    }
    // Fill out an XrInstanceCreateInfo structure and create an XrInstance.
    XrInstanceCreateInfo instanceCI{XR_TYPE_INSTANCE_CREATE_INFO};
    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = AI;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_activeAPILayers.size());
    instanceCI.enabledApiLayerNames = m_activeAPILayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
    instanceCI.enabledExtensionNames = m_activeInstanceExtensions.data();
    OPENXR_LOG(xrCreateInstance(&instanceCI, &m_xrInstance), "Failed to create Instance.");
    return XR_SUCCESS;
}
XrResult OpenXREnv::DestroyInstance()
{
    // Destroy the XrInstance.
    if(m_xrInstance != XR_NULL_HANDLE)
    {
        OPENXR_LOG(xrDestroyInstance(m_xrInstance), "Failed to destroy Instance.");
        m_xrInstance = XR_NULL_HANDLE;
    }
    return XR_SUCCESS;
}
void OpenXREnv::CreateDebugMessenger()
{
    // Check that "XR_EXT_debug_utils" is in the active Instance Extensions before creating an XrDebugUtilsMessengerEXT.
    if (IsStringInVector(m_activeInstanceExtensions, XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        m_debugUtilsMessenger = CreateOpenXRDebugUtilsMessenger(m_xrInstance);  // From OpenXRDebugUtils.h.
    }
}
void OpenXREnv::DestroyDebugMessenger()
{
    // Check that "XR_EXT_debug_utils" is in the active Instance Extensions before destroying the XrDebugUtilsMessengerEXT.
    if (m_debugUtilsMessenger != XR_NULL_HANDLE) {
        DestroyOpenXRDebugUtilsMessenger(m_xrInstance, m_debugUtilsMessenger);  // From OpenXRDebugUtils.h.
    }
}
XrResult OpenXREnv::GetInstanceProperties()
{
    // Get the instance's properties and log the runtime name and version.
    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    OPENXR_LOG(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get InstanceProperties.");
    XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - "
                                << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
                                << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
                                << XR_VERSION_PATCH(instanceProperties.runtimeVersion));
    return XR_SUCCESS;
}
XrResult OpenXREnv::GetSystemID()
{
    // Get the XrSystemId from the instance and the supplied XrFormFactor.
    XrSystemGetInfo systemGI{XR_TYPE_SYSTEM_GET_INFO};
    systemGI.formFactor = m_formFactor;
    OPENXR_LOG(xrGetSystem(m_xrInstance, &systemGI, &m_systemID), "Failed to get SystemID.");
    // Get the System's properties for some general information about the hardware and the vendor.
    OPENXR_LOG(xrGetSystemProperties(m_xrInstance, m_systemID, &m_systemProperties), "Failed to get SystemProperties.");
    return XR_SUCCESS;
}
XrResult OpenXREnv::GetEnvironmentBlendModes()
{
    // Retrieves the available blend modes. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t environmentBlendModeCount = 0;
    OPENXR_LOG(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemID, m_viewConfiguration, 0, &environmentBlendModeCount, nullptr),
               "Failed to enumerate EnvironmentBlend Modes.");
    m_environmentBlendModes.resize(environmentBlendModeCount);
    OPENXR_LOG(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemID, m_viewConfiguration, environmentBlendModeCount,
                                                &environmentBlendModeCount, m_environmentBlendModes.data()),
               "Failed to enumerate EnvironmentBlend Modes.");
    // Pick the first application supported blend mode supported by the hardware.
    for (const XrEnvironmentBlendMode &environmentBlendMode : m_applicationEnvironmentBlendModes) {
        if (std::find(m_environmentBlendModes.begin(), m_environmentBlendModes.end(), environmentBlendMode) != m_environmentBlendModes.end()) {
            m_environmentBlendMode = environmentBlendMode;
            break;
        }
    }
    if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM) {
        XR_TUT_LOG_ERROR("Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE.");
        m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    }
    std::cout << "Using environmentBlendMode " << (m_environmentBlendMode == 1 ? "opaque" : "additive") << '\n';
    return XR_SUCCESS;
}
XrResult OpenXREnv::GetViewConfigurationViews()
{
    // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationCount = 0;
    OPENXR_LOG(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, 0, &viewConfigurationCount, nullptr),
               "Failed to enumerate View Configurations.");
    m_viewConfigurations.resize(viewConfigurationCount);
    OPENXR_LOG(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, viewConfigurationCount, &viewConfigurationCount,
                                             m_viewConfigurations.data()),
               "Failed to enumerate View Configurations.");
    // Pick the first application supported View Configuration Type con supported by the hardware.
    for (const XrViewConfigurationType &viewConfiguration : m_applicationViewConfigurations) {
        if (std::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfiguration) != m_viewConfigurations.end()) {
            m_viewConfiguration = viewConfiguration;
            break;
        }
    }
    if (m_viewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM) {
        std::cerr << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO." << std::endl;
        m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }
    std::cout << "Using viewConfiguration " << (m_viewConfiguration == 2 ? "stereo" : "mono") << '\n';
    // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationViewCount = 0;
    OPENXR_LOG(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, 0, &viewConfigurationViewCount, nullptr),
               "Failed to enumerate ViewConfiguration Views.");
    m_viewConfigurationViews.resize(viewConfigurationViewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    OPENXR_LOG(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, viewConfigurationViewCount,
                                                 &viewConfigurationViewCount, m_viewConfigurationViews.data()),
               "Failed to enumerate ViewConfiguration Views.");
    return XR_SUCCESS;
}
void OpenXREnv::GetGraphicsAPI(std::shared_ptr<GraphicsAPI_Vulkan> api) {
    m_graphicsAPI = api;
}

XrResult OpenXREnv::CreateSession()
{
    // Create an XrSessionCreateInfo structure.
    XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};
    // Create a std::unique_ptr<GraphicsAPI_...> from the instance and system.
    // This call sets up a graphics API that's suitable for use with OpenXR.
    if (m_apiType != VULKAN){
        XR_TUT_LOG_ERROR("ERROR: Unknown Graphics API.");
        DEBUG_BREAK;
    }
    XrGraphicsBindingVulkanKHR graphicsBinding_{};
    graphicsBinding_                  = {XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};
    graphicsBinding_.instance         = m_graphicsAPI->instance;
    graphicsBinding_.physicalDevice   = m_graphicsAPI->physicalDevice;
    graphicsBinding_.device           = m_graphicsAPI->device;
    graphicsBinding_.queueFamilyIndex = m_graphicsAPI->queueFamilyIndex;  // first one must be a graphics queue
    graphicsBinding_.queueIndex       = m_graphicsAPI->queueIndex;
    // Fill out the XrSessionCreateInfo structure and create an XrSession.
    sessionCI.next        = &graphicsBinding_;
    sessionCI.createFlags = 0;
    sessionCI.systemId = m_systemID;
    OPENXR_LOG(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");
    return XR_SUCCESS;
}
XrResult OpenXREnv::DestroySession()
{
    // Destroy the XrSession.
    if(m_session != XR_NULL_HANDLE)
    {
        OPENXR_LOG(xrDestroySession(m_session), "Failed to destroy Session.");
        m_session = XR_NULL_HANDLE;
    }
    return XR_SUCCESS;
}

void OpenXREnv::PollEvents()
{
    // Poll OpenXR for a new event.
    XrEventDataBuffer eventData{XR_TYPE_EVENT_DATA_BUFFER};
    auto XrPollEvents = [&]() -> bool {
        eventData = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
    };
    while (XrPollEvents()) {
        switch (eventData.type) {
        // Log the number of lost events from the runtime.
        case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
            XrEventDataEventsLost *eventsLost = reinterpret_cast<XrEventDataEventsLost *>(&eventData);
            XR_TUT_LOG("OPENXR: Events Lost: " << eventsLost->lostEventCount);
            break;
        }
        // Log that an instance loss is pending and shutdown the application.
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
            XrEventDataInstanceLossPending *instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending *>(&eventData);
            XR_TUT_LOG("OPENXR: Instance Loss Pending at: " << instanceLossPending->lossTime);
            m_sessionRunning = false;
            m_applicationRunning = false;
            break;
        }
        // Log that the interaction profile has changed.
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
            XrEventDataInteractionProfileChanged *interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged *>(&eventData);
            XR_TUT_LOG("OPENXR: Interaction Profile changed for Session: " << interactionProfileChanged->session);
            if (interactionProfileChanged->session != m_session) {
                XR_TUT_LOG("XrEventDataInteractionProfileChanged for unknown Session");
                break;
            }
            break;
        }
        // Log that there's a reference space change pending.
        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
            XrEventDataReferenceSpaceChangePending *referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending *>(&eventData);
            XR_TUT_LOG("OPENXR: Reference Space Change pending for Session: " << referenceSpaceChangePending->session);
            if (referenceSpaceChangePending->session != m_session) {
               XR_TUT_LOG("XrEventDataReferenceSpaceChangePending for unknown Session");
                break;
            }
            break;
        }
        // Session State changes:
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            XrEventDataSessionStateChanged *sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged *>(&eventData);
            if (sessionStateChanged->session != m_session) {
                XR_TUT_LOG("XrEventDataSessionStateChanged for unknown Session");
                break;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
                // SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
                XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                sessionBeginInfo.primaryViewConfigurationType = m_viewConfiguration;
                OPENXR_CHECK(xrBeginSession(m_session, &sessionBeginInfo), "Failed to begin Session.");
                m_sessionRunning = true;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
                // SessionState is stopping. End the XrSession.
                OPENXR_CHECK(xrEndSession(m_session), "Failed to end Session.");
                m_sessionRunning = false;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
                // SessionState is exiting. Exit the application.
                m_sessionRunning = false;
                m_applicationRunning = false;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
                // SessionState is loss pending. Exit the application.
                // It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
                m_sessionRunning = false;
                m_applicationRunning = false;
            }
            // Store state for reference across the application.
            m_sessionState = sessionStateChanged->state;
            break;
        }
        default: {
            break;
        }
        }
    }
}
XrResult OpenXREnv::CreateReferenceSpace()
{
    // Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
    XrReferenceSpaceCreateInfo referenceSpaceCI{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    referenceSpaceCI.referenceSpaceType = m_refSpace;
    referenceSpaceCI.poseInReferenceSpace = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -2.0f}};
    OPENXR_LOG(xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_localSpace), "Failed to create ReferenceSpace.");
    std::cout << "Using Reference space: " << (m_refSpace == 2 ? "local" : "other") << '\n';
    return XR_SUCCESS;
}
XrResult OpenXREnv::DestroyReferenceSpace()
{
    // Destroy the reference XrSpace.
    if(m_localSpace != XR_NULL_HANDLE)
    {
        OPENXR_LOG(xrDestroySpace(m_localSpace), "Failed to destroy Space.");
        m_localSpace = XR_NULL_HANDLE;
    }
    return XR_SUCCESS;
}
XrResult OpenXREnv::CreateSwapchains()
{
    // Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
    uint32_t formatCount = 0;
    OPENXR_LOG(xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr), "Failed to enumerate Swapchain Formats");
    std::vector<int64_t> formats(formatCount);
    OPENXR_LOG(xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data()), "Failed to enumerate Swapchain Formats");

    //Resize the SwapchainInfo to match the number of view in the View Configuration.
    m_colorSwapchainInfos.resize(m_viewConfigurationViews.size());
    // Per view, create a color swapchain, and their associated image views.
    for (size_t i = 0; i < m_viewConfigurationViews.size(); i++) {
        SwapchainInfo &colorSwapchainInfo = m_colorSwapchainInfos[i];
        // Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
        // Color.
        XrSwapchainCreateInfo swapchainCI{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags  = XR_SWAPCHAIN_USAGE_TRANSFER_SRC_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.format = m_graphicsAPI->SelectColorSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = 1;  // Not Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
        swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        OPENXR_LOG(xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain), "Failed to create Color Swapchain");
        colorSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

        std::cout << "recommend w " << swapchainCI.width << "\nrecommend h " << swapchainCI.height << std::endl;
        auto recommendSamples = m_viewConfigurationViews[i].recommendedSwapchainSampleCount;
        if(recommendSamples > 1)
            std::cout << "recommend sample " << recommendSamples << ", but we only use sample=1." << std::endl;
        
        // Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
        uint32_t colorSwapchainImageCount = 0;
        OPENXR_LOG(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr),
                   "Failed to enumerate Color Swapchain Images.");
        std::cout << "\nswapchain image num " << colorSwapchainImageCount << std::endl;
        XrSwapchainImageBaseHeader* colorSwapchainImages =
            m_graphicsAPI->AllocateSwapchainImageData(colorSwapchainInfo.swapchain, GraphicsAPI::SwapchainType::COLOR,
                                                      colorSwapchainImageCount);
        OPENXR_LOG(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount,
                                              &colorSwapchainImageCount, colorSwapchainImages),
                   "Failed to enumerate Color Swapchain Images.");
        // Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
        for (uint32_t j = 0; j < colorSwapchainImageCount; j++) {
            GraphicsAPI::ImageViewCreateInfo imageViewCI;
            imageViewCI.image = m_graphicsAPI->GetSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCI.type           = GraphicsAPI::ImageViewCreateInfo::Type::RTV;
            imageViewCI.view           = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCI.format = colorSwapchainInfo.swapchainFormat;
            imageViewCI.aspect         = GraphicsAPI::ImageViewCreateInfo::Aspect::COLOR_BIT;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(m_graphicsAPI->CreateImageView(imageViewCI));
        }
    }
    return XR_SUCCESS;
}
XrResult OpenXREnv::DestroySwapchains()
{
    // Per view in the view configuration:
    for(size_t i = 0; i < m_viewConfigurationViews.size(); i++)
    {
        SwapchainInfo& colorSwapchainInfo = m_colorSwapchainInfos[i];

        // Destroy the color and depth image views from GraphicsAPI.
        for(void*& imageView : colorSwapchainInfo.imageViews)
        {
            if(imageView)
            {
                m_graphicsAPI->DestroyImageView(imageView);
            }
        }

        // Free the Swapchain Image Data.
        if(colorSwapchainInfo.swapchain)
        {
            m_graphicsAPI->FreeSwapchainImageData(colorSwapchainInfo.swapchain);
        }

        // Destroy the swapchains.
        if(colorSwapchainInfo.swapchain)
        {
            OPENXR_LOG(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain");
            colorSwapchainInfo.swapchain = XR_NULL_HANDLE;
        }
    }
    return XR_SUCCESS;
}
bool OpenXREnv::BeginFrame(RenderLayerInfo& renderLayerInfo) {
    // Get the XrFrameState for timing and rendering info.
    XrFrameState    frameState{XR_TYPE_FRAME_STATE};
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
    OPENXR_CHECK(xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");
    // Tell the OpenXR compositor that the application is beginning the frame.
    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    OPENXR_CHECK(xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");
    // Variables for rendering and layer composition.
    renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;
    // Check that the session is active and that we should render.
    bool sessionActive = (m_sessionState == XR_SESSION_STATE_SYNCHRONIZED || m_sessionState == XR_SESSION_STATE_VISIBLE
                          || m_sessionState == XR_SESSION_STATE_FOCUSED);
    return sessionActive && frameState.shouldRender;
}

void OpenXREnv::EndFrame(RenderLayerInfo& renderLayerInfo)
{
    for(uint32_t i = 0; i < renderLayerInfo.layerProjectionViews.size(); i++)
    {
        // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        OPENXR_CHECK(xrReleaseSwapchainImage(m_colorSwapchainInfos[i].swapchain, &releaseInfo),
                     "Failed to release Image back to the Color Swapchain");
    }
    // Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
    renderLayerInfo.layerProjection.layerFlags =
        XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.layerProjection.space     = m_localSpace;
    renderLayerInfo.layerProjection.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
    renderLayerInfo.layerProjection.views     = renderLayerInfo.layerProjectionViews.data();
    // Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime          = renderLayerInfo.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = m_environmentBlendMode;
    frameEndInfo.layerCount           = static_cast<uint32_t>(renderLayerInfo.layers.size());
    frameEndInfo.layers               = renderLayerInfo.layers.data();
    OPENXR_CHECK(xrEndFrame(m_session, &frameEndInfo), "Failed to end the XR Frame.");
}

void OpenXREnv::RenderFrame(RenderLayerInfo& renderLayerInfo)
{
}
bool OpenXREnv::RenderLayer(RenderLayerInfo& renderLayerInfo, VkCommandBuffer cmd, std::shared_ptr<GaussianSplatting> gsRenderer)
{
    // Locate the views from the view configuration within the (reference) space at the display time.
    std::vector<XrView> views(m_viewConfigurationViews.size(), {XR_TYPE_VIEW});
    XrViewState viewState{XR_TYPE_VIEW_STATE};  // Will contain information on whether the position and/or orientation is valid and/or tracked.
    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
    viewLocateInfo.viewConfigurationType = m_viewConfiguration;
    viewLocateInfo.displayTime           = renderLayerInfo.predictedDisplayTime;
    viewLocateInfo.space                 = m_localSpace;
    uint32_t viewCount                   = 0;
    XrResult result =
        xrLocateViews(m_session, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data());
    if(result != XR_SUCCESS)
    {
        XR_TUT_LOG("Failed to locate Views.");
        return false;
    }
    // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
    renderLayerInfo.layerProjectionViews.resize(viewCount, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW});
    // Per view in the view configuration:
    for(uint32_t i = 0; i < viewCount; i++)
    {
        SwapchainInfo& colorSwapchainInfo = m_colorSwapchainInfos[i];
        // Acquire and wait for an image from the swapchains.
        // Get the image index of an image in the swapchains.
        // The timeout is infinite.
        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex[i]),
                     "Failed to acquire Image from the Color Swapchian");
        XrSwapchainImageWaitInfo waitInfo = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout                  = XR_INFINITE_DURATION;
        OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");

        // Get the width and height and construct the viewport and scissors.
        const uint32_t& width    = m_viewConfigurationViews[i].recommendedImageRectWidth;
        const uint32_t& height   = m_viewConfigurationViews[i].recommendedImageRectHeight;
        GraphicsAPI::Viewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
        float           nearZ    = 0.09f;
        float           farZ     = 2000.0f;
        // Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
        // This also associates the swapchain image with this layer projection view.
        renderLayerInfo.layerProjectionViews[i]                    = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        renderLayerInfo.layerProjectionViews[i].pose               = views[i].pose;
        renderLayerInfo.layerProjectionViews[i].fov                = views[i].fov;
        renderLayerInfo.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.x      = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.y      = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.width  = static_cast<int32_t>(width);
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
        renderLayerInfo.layerProjectionViews[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering.
        // Compute the view-projection transform.
        // All matrices (including OpenXR's) are column-major, right-handed.
        XrMatrix4x4f_CreateProjectionFov(&cameraConstants.Proj, m_apiType, views[i].fov, nearZ, farZ);
        XrMatrix4x4f toView;
        XrVector3f   scale1m{1.0f, 1.0f, 1.0f};
        XrMatrix4x4f_CreateTranslationRotationScale(&toView, &views[i].pose.position, &views[i].pose.orientation, &scale1m);
        XrMatrix4x4f_InvertRigidBody(&cameraConstants.View, &toView);
        cameraConstants.pos = views[i].pose.position;
        cameraConstants.viewport = {(int)width, (int)height};
        // record render cmd
        gsRenderer->renderView(cmd, colorSwapchainInfo.imageViews[colorImageIndex[i]], (void*)&cameraConstants,
                               i == 1 ? m_graphicsAPI->GetSwapchainImage(colorSwapchainInfo.swapchain, colorImageIndex[i]) : nullptr);
    }
    return true;
}

CameraConstants* OpenXREnv::GetCamera() {
    return &cameraConstants;
}

VkImageView OpenXREnv::GetXRImageView(int view) {
    return (VkImageView)m_colorSwapchainInfos[view].imageViews[colorImageIndex[view]];
}

std::vector<std::string> OpenXREnv::GetInstanceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId)
{
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(m_xrInstance, systemId, 0, &extensionNamesSize, nullptr),
                 "Failed to get Vulkan Instance Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(m_xrInstance, systemId, extensionNamesSize, &extensionNamesSize,
                                                  extensionNames.data()),
                 "Failed to get Vulkan Instance Extensions.");

    std::stringstream        streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string              extension;
    while(std::getline(streamData, extension, ' '))
    {
        extensions.push_back(extension);
    }
    return extensions;
}
std::vector<std::string> OpenXREnv::GetDeviceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId)
{
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(m_xrInstance, systemId, 0, &extensionNamesSize, nullptr),
                 "Failed to get Vulkan Device Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(m_xrInstance, systemId, extensionNamesSize, &extensionNamesSize,
                                                extensionNames.data()),
                 "Failed to get Vulkan Device Extensions.");

    std::stringstream        streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string              extension;
    while(std::getline(streamData, extension, ' '))
    {
        extensions.push_back(extension);
    }
    return extensions;
}
