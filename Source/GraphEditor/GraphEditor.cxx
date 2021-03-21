#include <imgui.h>

#include "GraphEditor.h"

using namespace ImGui;

void
GE_Draw(void)
{
	static float f = 0.0f;
	static int counter = 0;

	Begin("Blyat");
	Text("Fuck seeples");
	SliderFloat("float", &f, 0.0f, 1.0f);

	if (Button("Button"))
		counter++;
	SameLine();
	Text("counter = %d", counter);

	Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / GetIO().Framerate, GetIO().Framerate);
	End();
}