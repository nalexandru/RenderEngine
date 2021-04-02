#ifndef _RE_BUFFER_H_
#define _RE_BUFFER_H_

#include <RenderEngine/Types.h>

// these values match VkBufferUsageFlagBits
enum ReBufferUsage
{
	RE_BU_TRANSFER_SRC		= 0x00000001,
	RE_BU_TRANSFER_DST		= 0x00000002,
	RE_BU_UNIFORM_BUFFER	= 0x00000010,
	RE_BU_STORAGE_BUFFER	= 0x00000020,
	RE_BU_VERTEX_BUFFER		= 0x00000040,
	RE_BU_INDEX_BUFFER		= 0x00000080,
	RE_BU_INDIRECT_BUFFER	= 0x00000100
};

struct ReBufferCreateInfo
{
	enum ReBufferUsage usage;
	enum ReMemoryType memoryType;
	uint64_t size;
};

#endif /* _RE_BUFFER_H_ */
