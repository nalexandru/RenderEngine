#include "RenderEngine_Internal.h"

uint32_t Re_frameId{ 0 };

void
Re_RenderScene(const struct ReScene *scene, const struct ReCameraInfo *ci, const struct ReRenderSettings *opt)
{
	(void)scene;
	(void)ci;
	(void)opt;

	uint32_t imageId{ Re_AcquireNextImage() };
	if (imageId == UINT32_MAX)
		return;

	vkWaitForFences(Re_device, 1, &Re_swapchain.fences[Re_frameId], VK_TRUE, UINT64_MAX);
	vkResetFences(Re_device, 1, &Re_swapchain.fences[Re_frameId]);

	VkCommandBuffer cmdBuffer{ Re_context.commandBuffers[Re_frameId] };

	vkResetCommandBuffer(cmdBuffer, 0);

	VkCommandBufferBeginInfo cbbi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkClearColorValue clearValue{};
	clearValue.float32[0] = 1.f;
	VkImageSubresourceRange clearRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	vkBeginCommandBuffer(cmdBuffer, &cbbi);

	Re_TransitionImageLayout(cmdBuffer, Re_swapchain.images[imageId], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkCmdClearColorImage(cmdBuffer, Re_swapchain.images[imageId], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &clearRange);
	Re_TransitionImageLayout(cmdBuffer, Re_swapchain.images[imageId], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	vkEndCommandBuffer(cmdBuffer);

	VkPipelineStageFlags waitStage{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &Re_swapchain.frameStart;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &Re_swapchain.frameEnd;
	vkQueueSubmit(Re_queue, 1, &submitInfo, Re_swapchain.fences[Re_frameId]);

	Re_Present(imageId);

	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}
