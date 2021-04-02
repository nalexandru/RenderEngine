#include <assert.h>
#include <stdlib.h>

#include "RenderEngine_Internal.h"

struct ReTexture *
Re_CreateTexture(const struct ReTextureCreateInfo *tci)
{
	struct ReTexture *tex{ (struct ReTexture *)calloc(1, sizeof(*tex)) };
	assert("Failed to allocate memory" && tex);

	tex->width = tci->width;
	tex->height = tci->height;
	tex->depth = tci->depth;

	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.extent = { tci->width, tci->height, tci->depth };
	imageInfo.mipLevels = tci->mipLevels;
	imageInfo.arrayLayers = tci->arrayLayers;
	imageInfo.samples = (VkSampleCountFlagBits)tci->samples;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = (VkImageUsageFlagBits)tci->usage;

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, tci->mipLevels, 0, tci->arrayLayers };

	switch (tci->type) {
	case RE_TEXTURE_2D:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	break;
	case RE_TEXTURE_3D:
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	break;
	case RE_TEXTURE_CUBE:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	break;
	}

	switch (tci->format) {
	case RE_TF_R8G8B8A8_UNORM: imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; break;
	case RE_TF_R8G8B8A8_UNORM_SRGB: imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB; break;
	case RE_TF_R16G16B16A16_SFLOAT: imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT; break;
	case RE_TF_R32G32B32A32_SFLOAT: imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
	case RE_TF_D32_SFLOAT: imageInfo.format = VK_FORMAT_D32_SFLOAT; break;
	case RE_TF_BC7_UNORM: imageInfo.format = VK_FORMAT_BC7_UNORM_BLOCK; break;
	case RE_TF_BC7_SRGB: imageInfo.format = VK_FORMAT_BC7_SRGB_BLOCK; break;
	}
	viewInfo.format = imageInfo.format;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	switch (tci->memoryType) {
	case RE_MT_GPU_LOCAL: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; break;
	case RE_MT_CPU_COHERENT: allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY; break;
	case RE_MT_CPU_VISIBLE: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; break;
	}


	assert("Failed to create image" &&
		vmaCreateImage(Re_allocator, &imageInfo, &allocInfo, &tex->image, &tex->memory, NULL) == VK_SUCCESS);

	viewInfo.image = tex->image;
	assert("Failed to create image view" &&
		vkCreateImageView(Re_device, &viewInfo, nullptr, &tex->view) == VK_SUCCESS);

	return tex;
}

void
Re_UploadTexture(struct ReTexture *tex, const void *data, uint64_t size)
{
	struct ReBufferCreateInfo stagingInfo{};
	stagingInfo.usage = (ReBufferUsage)(RE_BU_TRANSFER_SRC | RE_BU_TRANSFER_DST);
	stagingInfo.memoryType = RE_MT_CPU_COHERENT;
	stagingInfo.size = size;
	struct ReBuffer *staging{ Re_CreateBuffer(&stagingInfo) };
	assert(staging);

	VkCommandBuffer cmdBuff{ ReH_OneShotCommandBuffer() };

	Re_TransitionImageLayout(cmdBuff, tex->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VkBufferImageCopy region{};
	region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.imageExtent = { tex->width, tex->height, tex->depth };

	vkCmdCopyBufferToImage(cmdBuff, staging->buff, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	Re_TransitionImageLayout(cmdBuff, tex->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ReH_ExecuteCommandBuffer(cmdBuff);

	Re_DestroyBuffer(staging);

	tex->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void
Re_DestroyTexture(struct ReTexture *tex)
{
	vkDestroyImageView(Re_device, tex->view, nullptr);
	vkDestroyImage(Re_device, tex->image, nullptr);
	vmaFreeMemory(Re_allocator, tex->memory);
}

void
Re_TransitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkPipelineStageFlagBits src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = { aspect, 0, 1, 0, 1 };

	switch (barrier.oldLayout) {
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		src = VK_PIPELINE_STAGE_HOST_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		src = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	default:
		barrier.srcAccessMask = 0;
		break;
	}

	switch (barrier.newLayout) {
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	default:
		barrier.dstAccessMask = 0;
		break;
	}

	vkCmdPipelineBarrier(cmdBuffer, src, dst, 0, 0, NULL, 0, NULL, 1, &barrier);
}
