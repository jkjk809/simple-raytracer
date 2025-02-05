#include "Editor.h"
#include "Renderer.h"


class Application
{
public:
	Application();
	~Application();
	void createApp();
	void runApp();
	
private:
	void processInput();
	bool isRunning();
private:
	Renderer m_renderer;
	Editor m_editor;
	Keyboard m_keyboard; 
	Camera m_camera;
	Mouse m_mouse;

	float m_deltaTime;
	float m_lastFrame;

	bool running = true;
};