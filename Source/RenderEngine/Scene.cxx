#include <vector>

#include "RenderEngine_Internal.h"

using namespace std;

struct ReScene
{
	vector<ReModel *> models;
};

struct ReScene *
Re_CreateScene(const struct ReSceneCreateInfo *sci)
{
	return nullptr;
}

void
Re_AddModel(struct ReScene *s, struct ReModel *m)
{
	s->models.push_back(m);
}

void
Re_SetEnvironment(struct ReScene *s, struct ReEnvironmentInfo *e)
{
}

void
Re_DestroyScene(struct ReScene *s)
{
}

