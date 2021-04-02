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
	RE_TF_R8G8B8A8_UNORM_SRGB,
	RE_TF_R16G16B16A16_SFLOAT,
	RE_TF_R32G32B32A32_SFLOAT,
	RE_TF_D32_SFLOAT,
    RE_TF_BC7_UNORM,
    RE_TF_BC7_SRGB
};

enum ReMemoryType
{
	RE_MT_GPU_LOCAL,
	RE_MT_CPU_COHERENT,
	RE_MT_CPU_VISIBLE
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
