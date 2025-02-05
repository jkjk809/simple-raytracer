
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <chrono>

#include "Graphics/Texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/Shader.h"
#include "io/Mouse.h"
#include "io/Keyboard.h"
#include "io/Screen.h"
#include "io/Camera.h"
#include "Graphics/RenderSettings.h"
#include "Scene.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool init(const Camera& camera);
	void render(Camera& camera, Scene& scene);
	void newFrame();
	unsigned int getViewportTexture();
	Screen* getWindow();
	unsigned int accumulationTexture;
	int m_frames = 1;
private:
	void createFramebuffer();
	void generateTextureQuad();
	

private:
	unsigned int fbo, fbo2, postFBO, viewportTexture, currentTexture, rbo, brightAreasTexture;
	unsigned int finalFBO, finalTexture;

	unsigned int pFBO[2], pBuffer[2];

	std::unique_ptr<Screen> m_screen;
	std::unique_ptr<Shader> m_rayShader;
	std::unique_ptr<Shader> m_bloomShader;
	std::unique_ptr<Shader> m_postShader;

	Camera m_camera;
	Texture m_envMap;

	float startTime;

	unsigned int m_quadVAO;
	
	unsigned int ubo;
	
	float quadVertices[30] = {
		// positions         // texCoords
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f
	};

private:
	glm::vec3 previousCameraPosition;
	float previousYaw;
	float previousPitch;

};