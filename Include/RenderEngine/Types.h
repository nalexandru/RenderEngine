#ifndef _RE_TYPES_H_
#define _RE_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

enum ReCameraType
{
	RE_CAMERA_PERSPECTIVE,
	RE_CAMERA_ORTHOGRAPHIC
};

enum ReTextureType
{
	RE_TEXTURE_2D,
	RE_TEXTURE_3D,
	RE_TEXTURE_CUBE
};

enum ReTextureFormat
{
	RE_TF_R8G8B8A8_UNORM,
	RE_TF_R8G8B8A8_UNORM_SRGB
};

struct ReMesh;
struct ReModel;
struct ReScene;
struct ReBuffer;
struct ReVertex;
struct ReTexture;
struct ReMaterial;
struct ReCameraInfo;
struct RenderEngine;
struct ReRenderSettings;
struct ReModelCreateInfo;
struct ReEnvironmentInfo;
struct ReSceneCreateInfo;
struct ReRenderDeviceInfo;
struct ReTextureCreateInfo;
struct ReMaterialCreateInfo;

#endif /* _RE_TYPES_H_ */
