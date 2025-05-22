#include <DebugOutput.h>
#include <GraphicsAPI_Vulkan.h>
#include <OpenXRDebugUtils.h>

#include <xr_linear_algebra.h>

class MyApp;

struct RenderLayerInfo
{
  XrTime                                        predictedDisplayTime = 0;
  std::vector<XrCompositionLayerBaseHeader*>    layers;
  XrCompositionLayerProjection                  layerProjection = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
  std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
};
struct CameraConstants
{
  XrMatrix4x4f Proj;
  XrMatrix4x4f View;
  XrVector3f   pos;
  XrExtent2Di  viewport;
  XrVector4f   pad1;
  XrVector4f   pad2;
  XrVector4f   pad3;
};
class OpenXREnv
{
public:
  OpenXREnv(GraphicsAPI_Type apiType);
  ~OpenXREnv();

  void GetGraphicsAPI(std::shared_ptr<GraphicsAPI_Vulkan> api);

  XrResult InitVulkan();
  XrResult InitSession();
  XrResult CreateInstance();
  void CreateDebugMessenger();
  XrResult GetInstanceProperties();
  XrResult GetSystemID();
  XrResult GetViewConfigurationViews();
  XrResult GetEnvironmentBlendModes();
  XrResult CreateSession();
  XrResult CreateReferenceSpace();
  XrResult CreateSwapchains();

  void Destroy();
  XrResult DestroySwapchains();
  XrResult DestroyReferenceSpace();
  XrResult DestroySession();
  void DestroyDebugMessenger();
  XrResult DestroyInstance();

  bool RenderLayer(RenderLayerInfo& renderLayerInfo, VkCommandBuffer cmd);
  void RenderFrame(RenderLayerInfo& renderLayerInfo);
  bool BeginFrame(RenderLayerInfo& renderLayerInfo);
  void EndFrame(RenderLayerInfo& renderLayerInfo);

  void PollEvents();

  bool SessionRunning() const;
  bool AppRunning() const;

  CameraConstants* GetCamera();
  VkImageView      GetXRImageView(int view);
  XrInstance       GetXrInstance() { return m_xrInstance; }
  XrSystemId       GetXrSystemId() { return m_systemID; }

private:
  XrInstance               m_xrInstance               = XR_NULL_HANDLE;
  std::vector<const char*> m_activeAPILayers          = {};
  std::vector<const char*> m_activeInstanceExtensions = {};
  std::vector<std::string> m_apiLayers                = {};
  std::vector<std::string> m_instanceExtensions       = {};

  XrDebugUtilsMessengerEXT m_debugUtilsMessenger = XR_NULL_HANDLE;

  XrFormFactor       m_formFactor       = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
  XrSystemId         m_systemID         = {};
  XrSystemProperties m_systemProperties = {XR_TYPE_SYSTEM_PROPERTIES};

  GraphicsAPI_Type             m_apiType     = GraphicsAPI_Type::VULKAN;

  XrSession      m_session            = {};
  XrSessionState m_sessionState       = XR_SESSION_STATE_UNKNOWN;
  bool           m_applicationRunning = true;
  bool           m_sessionRunning     = false;

  std::vector<XrViewConfigurationType> m_applicationViewConfigurations = {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                                                          XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
  std::vector<XrViewConfigurationType> m_viewConfigurations;
  XrViewConfigurationType              m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
  std::vector<XrViewConfigurationView> m_viewConfigurationViews;

  struct SwapchainInfo
  {
    XrSwapchain        swapchain       = XR_NULL_HANDLE;
    int64_t            swapchainFormat = 0;
    std::vector<void*> imageViews;
  };
  std::vector<SwapchainInfo> m_colorSwapchainInfos = {};
  uint32_t                   colorImageIndex[2];

  std::vector<XrEnvironmentBlendMode> m_applicationEnvironmentBlendModes = {XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
                                                                            XR_ENVIRONMENT_BLEND_MODE_ADDITIVE};
  std::vector<XrEnvironmentBlendMode> m_environmentBlendModes            = {};
  XrEnvironmentBlendMode              m_environmentBlendMode             = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
  XrReferenceSpaceType                m_refSpace                         = XR_REFERENCE_SPACE_TYPE_LOCAL;

  XrSpace m_localSpace = XR_NULL_HANDLE;

  // In STAGE space, viewHeightM should be 0. In LOCAL space, it should be offset downwards, below the viewer's initial position.
  float m_viewHeightM = 1.5f;
  CameraConstants cameraConstants;

  PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = nullptr;
  PFN_xrGetVulkanInstanceExtensionsKHR   xrGetVulkanInstanceExtensionsKHR   = nullptr;
  PFN_xrGetVulkanDeviceExtensionsKHR     xrGetVulkanDeviceExtensionsKHR     = nullptr;
  PFN_xrGetVulkanGraphicsDeviceKHR       xrGetVulkanGraphicsDeviceKHR       = nullptr;
  std::vector<std::string>               GetInstanceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId);
  std::vector<std::string>               GetDeviceExtensionsForOpenXR(XrInstance m_xrInstance, XrSystemId systemId);
  std::vector<const char*>               activeInstanceLayers{};
  std::vector<const char*>               activeInstanceExtensions{};
  std::vector<const char*>               activeDeviceLayer{};
  std::vector<const char*>               activeDeviceExtensions{};

  std::shared_ptr<GraphicsAPI_Vulkan> m_graphicsAPI = nullptr;
};