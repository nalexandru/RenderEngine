#pragma once

#include <vulkan/vulkan.h>
#include <RenderEngine/RenderEngine.h>

#define RE_NUM_FRAMES	3

struct RenderContext
{
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffers[RE_NUM_FRAMES];
};

struct Swapchain
{
	VkSwapchainKHR sw;
	VkSemaphore frameStart, frameEnd;
	VkImageView *views;
	VkImage *images;
	uint32_t imageCount;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPresentModeKHR presentMode;
	VkFence fences[RE_NUM_FRAMES];
};

extern void *Re_window;
extern VkInstance Re_instance;
extern VkDevice Re_device;
extern VkPhysicalDevice Re_physicalDevice;
extern VkPhysicalDeviceProperties Re_physicalDeviceProperties;
extern VkPhysicalDeviceMemoryProperties Re_physicalDeviceMemoryProperties;
extern VkQueue Re_queue;
extern struct Swapchain Re_swapchain;
extern uint32_t Re_graphicsQueueFamily;
extern uint32_t Re_frameId;

// TODO: per-thread
extern struct RenderContext Re_context;

void Re_InitThread(void);
void Re_TermThread(void);

bool Re_InitDevice(const struct ReRenderDeviceInfo *info);
void Re_TermDevice(void);

bool Re_InitSwapchain(void);
uint32_t Re_AcquireNextImage(void);
bool Re_Present(uint32_t imageId);
void Re_ScreenResized(void);
void Re_TermSwapchain(void);

struct ReScene *Re_CreateScene(const struct ReSceneCreateInfo *sci);
void Re_AddModel(struct ReScene *s, struct ReModel *m);
void Re_SetEnvironment(struct ReScene *s, struct ReEnvironmentInfo *e);
void Re_DestroyScene(struct ReScene *s);

struct ReModel *Re_CreateModel(const struct ReModelCreateInfo *mci);
void Re_DestroyModel(struct ReModel *m);

struct ReTexture *Re_CreateTexture(const struct ReTextureCreateInfo *tci);
void Re_DestroyTexture(struct ReTexture *t);
void Re_TransitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout);

struct ReMaterial *Re_CreateMaterial(const struct ReMaterialCreateInfo *mci);
void Re_DestroyMaterial(struct ReMaterial *m);

void Re_RenderScene(const struct ReScene *scene, const struct ReCameraInfo *ci, const struct ReRenderSettings *opt);

