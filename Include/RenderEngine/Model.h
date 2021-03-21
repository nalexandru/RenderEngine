#ifndef _RE_MODEL_H_
#define _RE_MODEL_H_

#include <RenderEngine/Types.h>

struct ReVertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct ReMesh
{
	uint32_t firstVertex, vertexCount;
	uint32_t firstIndex, indexCount;
	const struct ReMaterial *material;
};

struct ReModelCreateInfo
{
	uint32_t vertexCount;
	const struct ReVertex *vertices;

	uint32_t indexCount;
	const uint32_t *indices;

	uint32_t meshCount;
	const uint32_t *meshes;
};

#endif /* _RE_MODEL_H_ */
