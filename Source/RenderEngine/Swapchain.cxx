#include <vector>
#include <assert.h>

#ifdef _WIN32
	#include <Windows.h>
#else
#	error "Platform not supported"
#endif

using namespace std;

#include "RenderEngine_Internal.h"

struct Swapchain Re_swapchain{};

static inline bool _Create(void);

bool
Re_InitSwapchain(void)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Re_physicalDevice, Re_swapchain.surface, &Re_swapchain.surfaceCapabilities);

	Re_swapchain.imageCount = 3;
	if (Re_swapchain.imageCount < Re_swapchain.surfaceCapabilities.minImageCount)
		Re_swapchain.imageCount = Re_swapchain.surfaceCapabilities.minImageCount;

	if (Re_swapchain.surfaceCapabilities.maxImageCount && Re_swapchain.imageCount > Re_swapchain.surfaceCapabilities.maxImageCount)
		Re_swapchain.imageCount = Re_swapchain.surfaceCapabilities.maxImageCount;

	uint32_t count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Re_physicalDevice, Re_swapchain.surface, &count, NULL);

	vector<VkSurfaceFormatKHR> formats{};
	formats.resize(count);

	vkGetPhysicalDeviceSurfaceFormatsKHR(Re_physicalDevice, Re_swapchain.surface, &count, formats.data());
	for (uint32_t i = 0; i < count; ++i) {
		if (formats[i].format == VK_FORMAT_R16G16B16A16_SFLOAT) {
			Re_swapchain.surfaceFormat = formats[i];
			break;
		}

		if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
			Re_swapchain.surfaceFormat = formats[i];
		else if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && Re_swapchain.surfaceFormat.format != VK_FORMAT_R8G8B8A8_UNORM)
			Re_swapchain.surfaceFormat = formats[i];
	}

	Re_swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;
/*	if (!E_GetCVarBln(L"Render_VerticalSync", false)->bln) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(Re_physicalDevice, surface, &count, NULL);

		VkPresentModeKHR *pm = Sys_Alloc(sizeof(*pm), count, MH_Transient);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Re_physicalDevice, surface, &count, pm);

		for (uint32_t i = 0; i < count; ++i) {
			if (pm[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				Re_swapchain.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if (Re_swapchain.presentMode != VK_PRESENT_MODE_MAILBOX_KHR && pm[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				Re_swapchain.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}*/

	VkSemaphoreCreateInfo sci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if (vkCreateSemaphore(Re_device, &sci, nullptr, &Re_swapchain.frameStart) != VK_SUCCESS)
		goto error;

	if (vkCreateSemaphore(Re_device, &sci, nullptr, &Re_swapchain.frameEnd) != VK_SUCCESS)
		goto error;

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
		vkCreateFence(Re_device, &fci, nullptr, &Re_swapchain.fences[i]);
	}

	if (!_Create())
		goto error;

	return true;

error:
	if (Re_swapchain.frameStart)
		vkDestroySemaphore(Re_device, Re_swapchain.frameStart, nullptr);

	if (Re_swapchain.frameEnd)
		vkDestroySemaphore(Re_device, Re_swapchain.frameEnd, nullptr);

	return false;
}

void
Re_TermSwapchain(void)
{
	vkDestroySemaphore(Re_device, Re_swapchain.frameEnd, nullptr);
	vkDestroySemaphore(Re_device, Re_swapchain.frameStart, nullptr);
	for (uint32_t i = 0; i < Re_swapchain.imageCount; ++i)
		vkDestroyImageView(Re_device, Re_swapchain.views[i], nullptr);
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		vkDestroyFence(Re_device, Re_swapchain.fences[i], nullptr);
	vkDestroySwapchainKHR(Re_device, Re_swapchain.sw, nullptr);

	free(Re_swapchain.images);
	free(Re_swapchain.views);
}

uint32_t
Re_AcquireNextImage(void)
{
	uint32_t imageId;
	VkResult rc = vkAcquireNextImageKHR(Re_device, Re_swapchain.sw, UINT64_MAX, Re_swapchain.frameStart, VK_NULL_HANDLE, &imageId);
	if (rc != VK_SUCCESS) {
		switch (rc) {
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			_Create();
		default: return UINT32_MAX;
		}
	}

/*	VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &Re_swapchain.frameSemaphore;
	waitInfo.pValues = &Re_swapchain.frameValues[Re_frameId];
	vkWaitSemaphores(Re_device, &waitInfo, UINT64_MAX);

	struct RenderContext *ctx;
	Rt_ArrayForEachPtr(ctx, &Vkd_contexts) {
		if (ctx->graphicsCmdBuffers[Re_frameId].count) {
			vkFreeCommandBuffers(Re_device, ctx->graphicsPools[Re_frameId],
				(uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data);
			vkResetCommandPool(Re_device, ctx->graphicsPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			Rt_ClearArray(&ctx->graphicsCmdBuffers[Re_frameId], false);
		}

		if (ctx->transferCmdBuffers[Re_frameId].count) {
			vkFreeCommandBuffers(Re_device, ctx->transferPools[Re_frameId],
				(uint32_t)ctx->transferCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->transferCmdBuffers[Re_frameId].data);
			vkResetCommandPool(Re_device, ctx->transferPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			Rt_ClearArray(&ctx->transferCmdBuffers[Re_frameId], false);
		}

		if (ctx->computeCmdBuffers[Re_frameId].count) {
			vkFreeCommandBuffers(Re_device, ctx->computePools[Re_frameId],
				(uint32_t)ctx->computeCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->computeCmdBuffers[Re_frameId].data);
			vkResetCommandPool(Re_device, ctx->computePools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			Rt_ClearArray(&ctx->computeCmdBuffers[Re_frameId], false);
		}
	}*/

	return imageId;
}

bool
Re_Present(uint32_t imageId)
{
/*	dev->frameValues[Re_frameId] = ++dev->semaphoreValue;

	uint64_t waitValues[] = { 0 };
	uint64_t signalValues[] = { dev->frameValues[Re_frameId], 0 };
	VkSemaphore wait[] = { Re_swapchain.frameStart };
	VkSemaphore signal[] = { dev->frameSemaphore, Re_swapchain.frameEnd };
	VkPipelineStageFlags waitMasks[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

	VkTimelineSemaphoreSubmitInfo timelineInfo =
	{
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.waitSemaphoreValueCount = 1,
		.pWaitSemaphoreValues = waitValues,
		.signalSemaphoreValueCount = 2,
		.pSignalSemaphoreValues = signalValues
	};
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineInfo,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait,
		.pWaitDstStageMask = waitMasks,
		.commandBufferCount = (uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count,
		.pCommandBuffers = (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data,
		.signalSemaphoreCount = 2,
		.pSignalSemaphores = signal
	};
	vkQueueSubmit(dev->graphicsQueue, 1, &si, VK_NULL_HANDLE);

	uint32_t imageId = (uint32_t)(uint64_t)image;*/

	VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &Re_swapchain.frameEnd;
	pi.swapchainCount = 1;
	pi.pSwapchains = &Re_swapchain.sw;
	pi.pImageIndices = &imageId;

	VkResult rc = vkQueuePresentKHR(Re_queue, &pi);
	switch (rc) {
	case VK_SUCCESS: return true;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		return _Create();
	default: return false;
	}

	return rc == VK_SUCCESS;
}

void
Re_ScreenResized(void)
{
	assert(_Create());
}

static inline bool
_Create(void)
{
	vkDeviceWaitIdle(Re_device);

	uint32_t width, height;
#ifdef _WIN32
	RECT rect;
	GetClientRect((HWND)Re_window, &rect);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
#else
#	error "Platform not supported"
#endif

	VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = Re_swapchain.surface;
	createInfo.minImageCount = Re_swapchain.imageCount;
	createInfo.imageFormat = Re_swapchain.surfaceFormat.format;
	createInfo.imageColorSpace = Re_swapchain.surfaceFormat.colorSpace;
	createInfo.imageExtent = { width, height };
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = false;
	createInfo.oldSwapchain = Re_swapchain.sw;
	createInfo.presentMode = Re_swapchain.presentMode;

	VkSwapchainKHR newSwapchain;
	if (vkCreateSwapchainKHR(Re_device, &createInfo, nullptr, &newSwapchain) != VK_SUCCESS)
		return false;

	Re_swapchain.sw = newSwapchain;

	uint32_t count;
	vkGetSwapchainImagesKHR(Re_device, Re_swapchain.sw, &count, NULL);

	if (Re_swapchain.imageCount != count || !Re_swapchain.images || !Re_swapchain.views) {
		void *ptr;

		ptr = realloc(Re_swapchain.images, count * sizeof(*Re_swapchain.images));
		assert(ptr); Re_swapchain.images = (VkImage *)ptr;

		ptr = realloc(Re_swapchain.views, count * sizeof(*Re_swapchain.views));
		assert(ptr); Re_swapchain.views = (VkImageView *)ptr;

		Re_swapchain.imageCount = count;
	}

	vkGetSwapchainImagesKHR(Re_device, Re_swapchain.sw, &Re_swapchain.imageCount, Re_swapchain.images);

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.format = Re_swapchain.surfaceFormat.format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
	viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.flags = 0;

	for (uint32_t i = 0; i < Re_swapchain.imageCount; ++i) {
		viewInfo.image = Re_swapchain.images[i];
		assert(vkCreateImageView(Re_device, &viewInfo, nullptr, &Re_swapchain.views[i]) == VK_SUCCESS);
	}

	return true;
}

