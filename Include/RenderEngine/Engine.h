#ifndef _RE_ENGINE_H_
#define _RE_ENGINE_H_

#include <RenderEngine/Types.h>

#define RE_RENDER_ENGINE_ID		0xB15B00B5
#define RE_RENDER_ENGINE_API	2

struct ReRenderSettings
{
	float gamma;
	float exposure;
};

struct ReRenderDeviceInfo
{
	char name[256];

	uint32_t vendorID, deviceID;
	uint64_t localMemorySize;

	struct {
		uint32_t maxTextureSize;
	} limits;

	struct {
		bool discrete;
		bool rayTracing;
		bool meshShader;
		bool unifiedMemory;
		bool bcTextureCompression;
		bool astcTextureCompression;
	} features;

	void *p;
};

struct RenderEngine
{
	void (*RenderScene)(const struct ReScene *scene, const struct ReCameraInfo *ci, const struct ReRenderSettings *opt);

	bool (*LoadGraphModule)(const char *path);

	struct ReScene *(*CreateScene)(const struct ReSceneCreateInfo *sci);
	void (*AddModel)(struct ReScene *s, struct ReModel *m);
	void (*SetEnvironment)(struct ReScene *s, struct ReEnvironmentInfo *e);
	void (*DestroyScene)(struct ReScene *s);

	struct ReModel *(*CreateModel)(const struct ReModelCreateInfo *mci);
	void (*DestroyModel)(struct ReModel *m);

	struct ReTexture *(*CreateTexture)(const struct ReTextureCreateInfo *tci);
	void (*UploadTexture)(struct ReTexture *tex, const void *data, uint64_t dataSize);
	void (*DestroyTexture)(struct ReTexture *tex);

	struct ReMaterial *(*CreateMaterial)(const struct ReMaterialCreateInfo *mci);
	void (*DestroyMaterial)(struct ReMaterial *m);

	void (*InitThread)(void);
	void (*TermThread)(void);

	bool (*Init)(void *window);
	void (*Term)(void);

	bool (*EnumerateDevices)(uint32_t *count, struct ReRenderDeviceInfo *info);

	bool (*InitDevice)(const struct ReRenderDeviceInfo *info);
	void (*TermDevice)(void);

	uint32_t id;
	uint32_t apiVersion;
};

typedef const struct RenderEngine *(*ReCreateRenderEngineProc)(void);

// The render engine dll will export a function called Re_CreateRenderEngine with the above signature.

#endif /* _RE_ENGINE_H_ */
