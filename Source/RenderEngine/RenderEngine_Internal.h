#pragma once

#include <mutex>
#include <assert.h>

#include <volk.h>
#include "vk_mem_alloc.h"
#include <RenderEngine/RenderEngine.h>

#define RE_NUM_FRAMES	3

#if defined(_MSC_VER)
#	define TLS __declspec(thread)
#elif defined(__GNUC__)
#	define TLS __thread
#endif

struct ReBuffer
{
	VkBuffer buff;
	void *ptr;
	VmaAllocation memory;
};

struct ReTexture
{
	VkImageView view;
	VkImageLayout layout;
	VkImage image;
	VmaAllocation memory;
	uint32_t width, height, depth;
};

struct RenderContext
{
	VkCommandPool commandPool, transientCommandPool;
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
extern VmaAllocator Re_allocator;
extern VkPhysicalDevice Re_physicalDevice;
extern VkPhysicalDeviceProperties Re_physicalDeviceProperties;
extern VkPhysicalDeviceMemoryProperties Re_physicalDeviceMemoryProperties;
extern VkQueue Re_queue;
extern struct Swapchain Re_swapchain;
extern uint32_t Re_graphicsQueueFamily;
extern uint32_t Re_frameId;
extern std::mutex Re_submitMutex;

extern TLS struct RenderContext Re_context;

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

struct ReBuffer *Re_CreateBuffer(const struct ReBufferCreateInfo *bci);
void Re_UploadBuffer(struct ReBuffer *buff, uint64_t offset, const void *data, uint64_t size);
void Re_DestroyBuffer(struct ReBuffer *buff);

struct ReTexture *Re_CreateTexture(const struct ReTextureCreateInfo *tci);
void Re_UploadTexture(struct ReTexture *tex, const void *data, uint64_t size);
void Re_DestroyTexture(struct ReTexture *tex);
void Re_TransitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout);

struct ReMaterial *Re_CreateMaterial(const struct ReMaterialCreateInfo *mci);
void Re_DestroyMaterial(struct ReMaterial *m);

void Re_RenderScene(const struct ReScene *scene, const struct ReCameraInfo *ci, const struct ReRenderSettings *opt);

// Helper functions
static inline VkCommandBuffer
ReH_OneShotCommandBuffer(void)
{
	VkCommandBuffer cmdBuff;

	VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = Re_context.transientCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	assert("Failed to allocate command buffer" && vkAllocateCommandBuffers(Re_device, &allocInfo, &cmdBuff) == VK_SUCCESS);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	assert("Failed to begin command buffer" && vkBeginCommandBuffer(cmdBuff, &beginInfo) == VK_SUCCESS);

	return cmdBuff;
}

static inline void
ReH_ExecuteCommandBuffer(VkCommandBuffer cmdBuff)
{
	VkFence fence;

	vkEndCommandBuffer(cmdBuff);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuff;

	VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(Re_device, &fenceInfo, nullptr, &fence);

	Re_submitMutex.lock();
	vkQueueSubmit(Re_queue, 1, &submitInfo, fence);
	Re_submitMutex.unlock();

	vkWaitForFences(Re_device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(Re_device, fence, nullptr);

	vkFreeCommandBuffers(Re_device, Re_context.transientCommandPool, 1, &cmdBuff);
}
