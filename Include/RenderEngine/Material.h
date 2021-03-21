#ifndef _RE_MATERIAL_H_
#define _RE_MATERIAL_H_

#include <RenderEngine/Types.h>

struct ReMaterialCreateInfo
{
	struct {
		float r, g, b, a;
	} parameters;

	const struct ReTexture *diffuseMap;
};

#endif /* _RE_MATERIAL_H_ */
