#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <RenderEngine/Types.h>

enum ReTextureUsage
{
	RE_TU_TRANSFER_SRC				= 0x00000001,
	RE_TU_TRANSFER_DST				= 0x00000002,
	RE_TU_SAMPLED					= 0x00000004,
	RE_TU_STORAGE					= 0x00000008,
	RE_TU_COLOR_ATTACHMENT			= 0x00000010,
	RE_TU_DEPTH_STENCIL_ATTACHMENT	= 0x00000020,
	RE_TU_TRANSIENT_ATTACHMENT		= 0x00000040,
	RE_TU_INPUT_ATTACHMENT			= 0x00000080
};

struct ReTextureCreateInfo
{
	enum ReTextureType type;
	enum ReTextureUsage usage;
	enum ReTextureFormat format;
	enum ReMemoryType memoryType;
	uint32_t width, height, depth, mipLevels, arrayLayers, samples;
};

#endif /* _RE_TEXTURE_H_ */