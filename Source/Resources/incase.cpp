#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imconfig.h"

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/Shader.h"
#include "io/Mouse.h"
#include "io/Keyboard.h"
#include "io/Screen.h"
#include "io/Camera.h"
#include "Scene.h"

Screen screen;
Camera camera(glm::vec3(0.0, 0.0, 2.0));
Mouse mouse(&camera);
Scene scene;

void processInput();

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float fps;
unsigned int fbo, viewportTexture, rbo;
void createFramebuffer();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
	if (!screen.init())
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	screen.setParameters();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	//ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(screen.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	Shader rayShader("Source\\Resources\\ray.vert", "Source\\Resources\\ray.frag");
	rayShader.use();



	float quadVertices[] = {
		// positions        // texCoords
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glm::mat4 model = glm::mat4(1.0f);
	rayShader.setMat4("model", model);
	glm::vec3 lightDir(1.0, -0.84, -1.54);

	glm::mat4 inverseView = glm::mat4(1.0f);

	glm::vec3 sphereColor = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 backgroundColor = glm::vec3(0.0);
	createFramebuffer();
	float radius = 0.5f;
	while (!screen.shouldClose())
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		fps = 1.0f / deltaTime;

		processInput();
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		screen.update();
		glm::mat4 view = camera.GetViewMatrix();
		inverseView = glm::inverse(view);
		rayShader.setMat4("invView", inverseView);
		rayShader.setVec3("rayOrigin", camera.getPosition());
		rayShader.setVec2("screenResolution", glm::vec2(screen.SCR_WIDTH, screen.SCR_HEIGHT));
		rayShader.setVec3("lightDir", lightDir);
		rayShader.setVec3("sphereColor", sphereColor);
		rayShader.setVec3("backgroundColor", backgroundColor);
		rayShader.setFloat("radius", radius);
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), static_cast<float>(screen.SCR_WIDTH) / static_cast<float> (screen.SCR_HEIGHT), 0.1f, 100.0f);
		glm::mat4 inverseProj = glm::inverse(projection);


		rayShader.setMat4("invProj", inverseProj);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screen.SCR_WIDTH, screen.SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGui::DockSpace(ImGui::GetID("DockSpace"));
		ImGui::End();

		ImGui::Begin("SceneGraph");
		if (ImGui::Button("Add Sphere")) {
			Scene::Sphere newSphere;
			scene.addSphere(newSphere);
		}

		for (size_t i = 0; i < scene.getSpheres().size(); i++)
		{
			Scene::Sphere& sphere = scene.getSpheres()[i];
			if (ImGui::TreeNode("Entity"))
			{
				ImGui::SliderFloat3("Position", sphere.position, -10.0f, 10.0f);
				ImGui::ColorEdit3("Color", sphere.albedo);
				ImGui::DragFloat("Radius", &sphere.radius);
				ImGui::TreePop();
			}
		}
		ImGui::End();

		ImGui::Begin("Settings");
		ImGui::Text("Render time: %.3f ms", deltaTime * 1000.0f);
		if (ImGui::CollapsingHeader("Settings"))
		{
			ImGui::SliderFloat3("Light Direction", &lightDir[0], -10.0f, 10.0f, "%.6f");
			ImGui::ColorEdit3("SphereColor", &sphereColor[0]);
			ImGui::ColorEdit3("BackgroundColor", &backgroundColor[0]);
			ImGui::DragFloat("Sphere Radius", &radius, 0.01f, -3.0f, 3.0f);
		}
		ImGui::End();

		ImGui::Begin("Scene");
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		float contentRegionAspectRatio = viewportSize.x / viewportSize.y;
		float aspectRatio = static_cast<float>(800) / static_cast<float>(600);

		if (contentRegionAspectRatio > aspectRatio)
		{
			float imageWidth = viewportSize.y * aspectRatio;
			float xPadding = (viewportSize.x - imageWidth) / 2;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPadding);
			ImGui::Image((ImTextureID)(intptr_t)viewportTexture, ImVec2(imageWidth, viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
		}
		// Scale the image vertically if the content region is taller than the image
		else
		{
			float imageHeight = viewportSize.x / aspectRatio;
			float yPadding = (viewportSize.y - imageHeight) / 2;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yPadding);
			ImGui::Image((ImTextureID)(intptr_t)viewportTexture, ImVec2(viewportSize.x, imageHeight), ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		screen.newFrame();
	}
}

void processInput()
{

	if (Keyboard::key(GLFW_KEY_ESCAPE))
		screen.setShouldClose(true);
	const float cameraSpeed = 1.0f * deltaTime; // adjust accordingly
	if (Keyboard::key(GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, cameraSpeed);
	if (Keyboard::key(GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, cameraSpeed);
	if (Keyboard::key(GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, cameraSpeed);

	if (Keyboard::key(GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, cameraSpeed);
	if (false)
		camera.Position.y = 1.0f;
}

void createFramebuffer() {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create a color attachment texture
	glGenTextures(1, &viewportTexture);
	glBindTexture(GL_TEXTURE_2D, viewportTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen.SCR_WIDTH, screen.SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);

	// Create a renderbuffer object for depth and stencil attachment (optional)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen.SCR_WIDTH, screen.SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}