#include <assert.h>
#include <stdlib.h>

#include "RenderEngine_Internal.h"

struct ReBuffer *
Re_CreateBuffer(const struct ReBufferCreateInfo *bci)
{
	struct ReBuffer *buff{ (struct ReBuffer *)calloc(1, sizeof(*buff)) };
	assert("Failed to allocate memory" && buff);

	VkBufferCreateInfo buffInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffInfo.size = bci->size;
	buffInfo.usage = (VkBufferUsageFlagBits)bci->usage;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	switch (bci->memoryType) {
	case RE_MT_GPU_LOCAL: allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; break;
	case RE_MT_CPU_COHERENT: allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY; allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; break;
	case RE_MT_CPU_VISIBLE: allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU; allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; break;
	}

	VmaAllocationInfo info{};
	assert("Failed to create buffer" &&
		vmaCreateBuffer(Re_allocator, &buffInfo, &allocInfo, &buff->buff, &buff->memory, &info) == VK_SUCCESS);

	buff->ptr = info.pMappedData;

	return buff;
}

void
Re_UploadBuffer(struct ReBuffer *buff, uint64_t offset, const void *data, uint64_t size)
{
	if (buff->ptr) {
		memcpy((uint8_t *)buff->ptr + offset, data, size);
	} else {
		struct ReBuffer *stagingBuffer{ nullptr };
		VkCommandBuffer cmdBuff{ ReH_OneShotCommandBuffer() };
		
		if (size > UINT16_MAX) {
			struct ReBufferCreateInfo stagingInfo{};
			stagingInfo.usage = (ReBufferUsage)(RE_BU_TRANSFER_SRC | RE_BU_TRANSFER_DST);
			stagingInfo.memoryType = RE_MT_CPU_COHERENT;
			stagingInfo.size = size;
			stagingBuffer = Re_CreateBuffer(&stagingInfo);
			assert(stagingBuffer);

			memcpy(stagingBuffer->ptr, data, size);

			VkBufferCopy region{ 0, offset, size };
			vkCmdCopyBuffer(cmdBuff, stagingBuffer->buff, buff->buff, 1, &region);
		} else {
			vkCmdUpdateBuffer(cmdBuff, buff->buff, offset, size, data);
		}

		ReH_ExecuteCommandBuffer(cmdBuff);

		if (stagingBuffer)
			Re_DestroyBuffer(stagingBuffer);
	}
}

void
Re_DestroyBuffer(struct ReBuffer *buff)
{
	vkDestroyBuffer(Re_device, buff->buff, nullptr);
	vmaFreeMemory(Re_allocator, buff->memory);
}

