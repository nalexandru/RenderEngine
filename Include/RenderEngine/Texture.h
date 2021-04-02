#ifndef _RE_TEXTURE_H_
#define _RE_TEXTURE_H_

#include <RenderEngine/Types.h>

struct ReTextureCreateInfo
{
	enum ReTextureType type;
	enum ReTextureFormat format;	
	uint32_t width, height, depth, mipLevels, arrayLayers, samples;
};


#endif /* _RE_TEXTURE_H_ */