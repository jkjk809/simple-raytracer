#include "Renderer.h"

Renderer::Renderer()
	:fbo(0), viewportTexture(0), rbo(0), m_screen(std::make_unique<Screen>()){}

Renderer::~Renderer()
{
}
bool Renderer::init(const Camera& camera)
{
	startTime = static_cast<float>(glfwGetTime());

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

	if (!m_screen->init())
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	m_camera = camera;

	m_screen->setParameters();
	
	m_rayShader = std::make_unique<Shader>("Source\\Resources\\ray.vert", "Source\\Resources\\ray.frag");
	m_bloomShader = std::make_unique<Shader>("Source\\Resources\\ray.vert", "Source\\Resources\\bloom.frag");
	m_postShader = std::make_unique<Shader>("Source\\Resources\\ray.vert", "Source\\Resources\\post.frag");

	createFramebuffer();
	generateTextureQuad();
	std::vector<std::string> faces = {
		"Source/Graphics/tokyo/posx.jpg",
		"Source/Graphics/tokyo/negx.jpg",
		"Source/Graphics/tokyo/posy.jpg",
		"Source/Graphics/tokyo/negy.jpg",
		"Source/Graphics/tokyo/posz.jpg",
		"Source/Graphics/tokyo/negz.jpg"
	};

	
	m_envMap.loadCubemap(faces);
	m_envMap.bind(0);

	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, 10 * sizeof(Scene::Sphere), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	std::cout << "renderer initialized" << std::endl;
	return true;
}

void Renderer::render(Camera& camera, Scene& scene)
{
	

	float currentTime = static_cast<float>(glfwGetTime());
	float elsapsedTime = currentTime - startTime;

	glm::vec3 newCameraPosition = camera.getPosition();
	float newYaw = camera.Yaw;
	float newPitch = camera.Pitch;

	// Check if the camera has changed
	bool cameraChanged = (newCameraPosition != m_camera.getPosition() ||
		newYaw != m_camera.Yaw ||
		newPitch != m_camera.Pitch);

	
	if (cameraChanged) {
		m_frames = 1; 
		m_camera = camera; 
	}

	bool useFramebuffer1 = (m_frames % 2 == 0);
	unsigned int currentFramebuffer = useFramebuffer1 ? fbo : fbo2;
	unsigned int lastTexture = useFramebuffer1 ? accumulationTexture : viewportTexture;
	currentTexture = useFramebuffer1 ? viewportTexture : accumulationTexture;
	

	m_camera = camera;
	glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightAreasTexture, 0);

	glViewport(0, 0, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	m_screen->update();
	 
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_screen->SCR_WIDTH) / static_cast<float> (m_screen->SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 inverseProj = glm::inverse(projection);
	glm::mat4 view = m_camera.GetViewMatrix();
	glm::mat4 inverseView = glm::inverse(view);


	const auto& settings = scene.getRenderSettings();

	m_rayShader->use();
	
	m_rayShader->setMat4("invProj", inverseProj);
	m_rayShader->setMat4("invView", inverseView);
	m_rayShader->setVec3("rayOrigin", m_camera.getPosition());
	m_rayShader->setVec3("lightDir", settings.lightDir);
	m_rayShader->setVec3("sphereColor", glm::vec3(0.2, 0.2, 0.2));
	m_rayShader->setVec3("backgroundColor", settings.backgroundColor);
	m_rayShader->setFloat("time", elsapsedTime);
	m_rayShader->setInt("envMap", 0);
	m_rayShader->setInt("frameCount", m_frames);
	
	const auto& spheres = scene.getSpheres();
	std::cout << spheres.size() << std::endl;

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, spheres.size() * sizeof(Scene::Sphere), spheres.data());
	m_rayShader->setInt("sphereCount", spheres.size());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lastTexture);
	m_rayShader->setInt("accumulatedTexture", 1);

	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	// BLOOM
	// horizontal and vertical gauussian blur.

	
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, brightAreasTexture);

	/*m_bloomShader->use();
	m_bloomShader->setInt("brightTexture", 1);

	bool horizontal = true, firstIt = true;
	int amount = 20;
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pFBO[horizontal]);
		m_bloomShader->setBool("horizontal", horizontal);
		if (firstIt)
		{
			glBindTexture(GL_TEXTURE_2D, brightAreasTexture);
			firstIt = false;
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, pBuffer[!horizontal]);
		}

		glBindVertexArray(m_quadVAO);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		horizontal = !horizontal;
	}*/

	m_postShader->use();
	m_postShader->setInt("originTexture", 0);
	// currently no bloom used
	m_postShader->setInt("bloomTexture", 1);

	glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
	glBindVertexArray(m_quadVAO);
	glDisable(GL_DEPTH_TEST);
	//glBindTexture(GL_TEXTURE_2D, pBuffer[!horizontal]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentTexture);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	m_frames++;
	//glViewport(0, 0, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
}

void Renderer::newFrame()
{
	m_screen->newFrame();
}
unsigned int Renderer::getViewportTexture()
{
	return finalTexture;
}

void Renderer::generateTextureQuad()
{
	unsigned int quadVBO;
	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void Renderer::createFramebuffer()
{
	glGenFramebuffers(1, &fbo);
	glGenFramebuffers(1, &fbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

	glGenTextures(1, &brightAreasTexture);
	glBindTexture(GL_TEXTURE_2D, brightAreasTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	

	glGenTextures(1, &accumulationTexture);
	glBindTexture(GL_TEXTURE_2D, accumulationTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulationTexture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &viewportTexture);
	glBindTexture(GL_TEXTURE_2D, viewportTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// bloom texture and pingpong buffers
	glGenFramebuffers(2, pFBO);
	glGenTextures(2, pBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pBuffer[i], 0);
	}
	// previous frame


	//POST FBO
	glGenFramebuffers(1, &finalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);

	// Create a texture to hold the final output
	glGenTextures(1, &finalTexture);
	glBindTexture(GL_TEXTURE_2D, finalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTexture, 0);

	// Check framebuffer status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Final framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screen->SCR_WIDTH, m_screen->SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	
}

Screen* Renderer::getWindow()
{
	return m_screen.get();
}