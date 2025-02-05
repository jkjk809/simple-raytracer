#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imconfig.h"
#include "io/Screen.h"
#include "Scene.h"
#include <string>


class Editor
{
public:
	Editor();
	void setupImGui(Screen* screen);
	void onUpdate(unsigned int const& viewportTexture, const float& deltaTime);
	Scene& getScene();
public: 
	
private:
	void newFrame();
	void renderSceneGraph();
	void renderSettings();
	void renderScene(unsigned int const& viewportTexture);

private:
	ImGuiWindowFlags m_windowFlags;
	Scene m_scene;
	Screen* m_screen;
	float m_deltaTime;
};