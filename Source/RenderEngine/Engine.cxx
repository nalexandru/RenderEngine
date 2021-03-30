#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <X11/Xlib.h>

Display *X11_display;
#endif

#include "RenderEngine_Internal.h"

using namespace std;

void *Re_window;
VkInstance Re_instance;
struct RenderContext Re_context;

static bool _Init(void *window);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *count, struct ReRenderDeviceInfo *info);
static bool _LoadGraphModule(const char *path);

static struct RenderEngine _re =
{
	Re_RenderScene,

	_LoadGraphModule,

	Re_CreateScene,
	Re_AddModel,
	Re_SetEnvironment,
	Re_DestroyScene,

	Re_CreateModel,
	Re_DestroyModel,

	Re_CreateTexture,
	Re_DestroyTexture,

	Re_CreateMaterial,
	Re_DestroyMaterial,

	Re_InitThread,
	Re_TermThread,

	_Init,
	_Term,

	_EnumerateDevices,

	Re_InitDevice,
	Re_TermDevice,

	RE_RENDER_ENGINE_ID,
	RE_RENDER_ENGINE_API
};

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif
extern "C" EXPORT const struct RenderEngine *Re_CreateRenderEngine(void) { return &_re; }

static bool
_Init(void *window)
{
	assert("Failed to initialize volk" && volkInitialize() == VK_SUCCESS);

	VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.pApplicationName = "RenderEngine";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pEngineName = "RenderEngine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.pApplicationInfo = &applicationInfo;

	vector<const char *> instLayers{};
	instLayers.push_back("VK_LAYER_KHRONOS_validation");

	instanceInfo.enabledLayerCount = (uint32_t)instLayers.size();
	instanceInfo.ppEnabledLayerNames = instLayers.data();

	vector<const char *> instExtensions{};

	instExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

#if defined(_WIN32)
	instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
#else
	instExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

	instanceInfo.enabledExtensionCount = (uint32_t)instExtensions.size();
	instanceInfo.ppEnabledExtensionNames = instExtensions.data();

	if (vkCreateInstance(&instanceInfo, nullptr, &Re_instance) != VK_SUCCESS)
		return false;

	volkLoadInstance(Re_instance);

#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR surfaceInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.hinstance = GetModuleHandle(NULL);
	surfaceInfo.hwnd = (HWND)window;

	if (vkCreateWin32SurfaceKHR(Re_instance, &surfaceInfo, nullptr, &Re_swapchain.surface) != VK_SUCCESS)
		return false;
#elif defined(__APPLE__)
#else
	X11_display = XOpenDisplay(NULL);
	
    VkXlibSurfaceCreateInfoKHR surfaceInfo{ VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR };
    surfaceInfo.dpy = X11_display;
	surfaceInfo.window = (Window)window;
    
    if (vkCreateXlibSurfaceKHR(Re_instance, &surfaceInfo, nullptr, &Re_swapchain.surface) != VK_SUCCESS)
        return false;
#endif

	Re_window = window;

	return true;
}

static void
_Term(void)
{
	vkDestroySurfaceKHR(Re_instance, Re_swapchain.surface, nullptr);
	vkDestroyInstance(Re_instance, nullptr);
}

void
Re_InitThread(void)
{
	struct RenderContext *ctx = &Re_context;

	VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = Re_graphicsQueueFamily;
	
	vkCreateCommandPool(Re_device, &poolInfo, nullptr, &ctx->commandPool);
	
	VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = ctx->commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = RE_NUM_FRAMES;
	vkAllocateCommandBuffers(Re_device, &allocateInfo, ctx->commandBuffers);
}

void
Re_TermThread(void)
{
	vkFreeCommandBuffers(Re_device, Re_context.commandPool, RE_NUM_FRAMES, Re_context.commandBuffers);
	vkDestroyCommandPool(Re_device, Re_context.commandPool, nullptr);
}

static bool
_EnumerateDevices(uint32_t *count, struct ReRenderDeviceInfo *info)
{
	if (!*count || !info)
		return vkEnumeratePhysicalDevices(Re_instance, count, NULL) == VK_SUCCESS;

	VkPhysicalDevice *dev = (VkPhysicalDevice *)calloc(sizeof(VkPhysicalDevice), *count);
	if (!dev)
		return false;

	if (vkEnumeratePhysicalDevices(Re_instance, count, dev) != VK_SUCCESS)
		return false;

	VkPhysicalDeviceProperties props;
	VkPhysicalDeviceMemoryProperties memProps;

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT edsFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &edsFeatures };
	VkPhysicalDeviceMeshShaderFeaturesNV msFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, &rtFeatures };
	VkPhysicalDeviceVulkan12Features vk12Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &msFeatures };
	VkPhysicalDeviceVulkan11Features vk11Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &vk12Features };
	VkPhysicalDeviceFeatures2 features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &vk11Features };

	for (uint32_t i = 0; i < *count; ++i) {
		bool present = false;
		uint32_t familyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(dev[i], &familyCount, NULL);
		for (uint32_t j = 0; j < familyCount; ++j)
#if defined(_WIN32)
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(dev[i], j))
#elif defined(__APPLE__)
#else
			if (1)
#endif
				present = true;

		if (!present)
			continue;

		vkGetPhysicalDeviceProperties(dev[i], &props);
		vkGetPhysicalDeviceFeatures2(dev[i], &features);

		// check requirements
		if (!features.features.fullDrawIndexUint32 || !features.features.samplerAnisotropy)
			continue;

		if (!vk12Features.imagelessFramebuffer || !vk12Features.descriptorIndexing ||
			!vk12Features.descriptorBindingPartiallyBound || !vk12Features.timelineSemaphore ||
			!vk12Features.shaderSampledImageArrayNonUniformIndexing || !vk12Features.runtimeDescriptorArray ||
			!vk12Features.descriptorBindingSampledImageUpdateAfterBind)
			continue;

		if (!edsFeatures.extendedDynamicState)
			continue;

		snprintf(info[i].name, sizeof(info[i].name), "%s", props.deviceName);

		info[i].features.meshShader = msFeatures.meshShader;
		info[i].features.rayTracing = rtFeatures.rayTracingPipeline && vk12Features.bufferDeviceAddress;
		info[i].features.discrete = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		info[i].features.bcTextureCompression = features.features.textureCompressionBC;
		info[i].features.astcTextureCompression = false; /* do we want mobile ? */

		info[i].limits.maxTextureSize = props.limits.maxImageDimension2D;

		info[i].localMemorySize = 0;
		vkGetPhysicalDeviceMemoryProperties(dev[i], &memProps);
		for (uint32_t j = 0; j < memProps.memoryHeapCount; ++j)
			if (memProps.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				info[i].localMemorySize += memProps.memoryHeaps[j].size;

		// this might not be 100% correct
		info[i].features.unifiedMemory = memProps.memoryHeapCount == 1;

		info[i].p = dev[i];
	}

	return true;
}

// Platform specific code

#ifdef _WIN32
#include <Windows.h>

static bool
_LoadGraphModule(const char *path)
{
	static HMODULE graphModule = nullptr;

	if (graphModule)
		FreeLibrary(graphModule);

	graphModule = LoadLibraryA(path);
	if (!graphModule)
		return false;

	// initialize graph

	return true;
}
#else
#include <dlfcn.h>

static bool
_LoadGraphModule(const char *path)
{
	static void *graphModule = nullptr;
	
	if (graphModule)
		dlclose(graphModule);
	
	graphModule = dlopen(path, RTLD_NOW);
	if (!graphModule)
		return false;
	
	// initialize graph
	
	return true;
}
#endif
