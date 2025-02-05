#include "Application.h"
Application::Application()
	:m_mouse(&m_camera), m_camera(glm::vec3(0.0, 0.0, 2.0))
{
}
Application::~Application()
{
}

void Application::createApp()
{
	m_renderer.init(m_camera);
	m_editor.setupImGui(m_renderer.getWindow());
	std::cout << "app created" << std::endl;
}

void Application::runApp()
{
	
	while (isRunning())
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		processInput();
		m_renderer.render(m_camera, m_editor.getScene());
		m_editor.onUpdate(m_renderer.getViewportTexture(), m_deltaTime);
		m_renderer.newFrame();
	}
}

bool Application::isRunning()
{
	return running;
}
void Application::processInput()
{
	if (Keyboard::key(GLFW_KEY_ESCAPE))
	{
		running = false;
	}

	const float cameraSpeed = 1.0f * m_deltaTime; 
	if (Keyboard::key(GLFW_KEY_W) == GLFW_PRESS)
	{
		m_camera.ProcessKeyboard(FORWARD, cameraSpeed);
	}
	if (Keyboard::key(GLFW_KEY_S) == GLFW_PRESS)
	{
		m_camera.ProcessKeyboard(BACKWARD, cameraSpeed);
	}
	if (Keyboard::key(GLFW_KEY_A) == GLFW_PRESS)
	{
		m_camera.ProcessKeyboard(LEFT, cameraSpeed);
	}
	if (Keyboard::key(GLFW_KEY_D) == GLFW_PRESS)
	{
		m_camera.ProcessKeyboard(RIGHT, cameraSpeed);
	}
	if (Keyboard::key(GLFW_KEY_T) == GLFW_PRESS)
	{
		m_renderer.m_frames = 1;
	}
	if (Keyboard::key(GLFW_KEY_Y) == GLFW_PRESS)
	{
		m_camera.ProcessKeyboard(RIGHT, cameraSpeed);
	}
}
