#ifndef _RE_SCENE_H_ 
#define _RE_SCENE_H_

#include <RenderEngine/Types.h>

struct ReCameraInfo
{
	struct {
		float x, y, z;
	} position;

	struct {
		float x, y, z;
	} eye;

	struct {
		float x, y, z;
	} up;

	enum ReCameraType type;
};

struct ReEnvironmentInfo
{
	const struct ReTexture *map;
};

struct ReSceneCreateInfo
{
	const struct ReEnvironmentInfo *environmentInfo;
};

#endif /* _RE_SCENE_H_ */
