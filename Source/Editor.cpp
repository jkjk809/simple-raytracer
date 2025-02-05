#include "Editor.h"
#include <iostream>
Editor::Editor()
{
}

void Editor::setupImGui(Screen* screen)
{
	m_screen = screen;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	//ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(screen->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	m_windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove 
		| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	//init scene

	Scene::Sphere floorSphere;
	floorSphere.position[0] = 0.0f;
	floorSphere.position[1] = -100.0f;
	floorSphere.position[2] = 0.0f;
	floorSphere.radius = 100.0f;
	floorSphere.roughness = 0.160f;
	floorSphere.albedo[0] = 0.388;
	floorSphere.albedo[1] = 0.267;
	floorSphere.albedo[2] = 0.161;

	Scene::Sphere sphere;
	sphere.position[0] = 0.0f;
	sphere.position[1] = 0.5f;
	sphere.position[2] = 0.0f;
	sphere.radius = 0.5f;
	sphere.roughness = 0.25f;
	sphere.albedo[0] = 0.129;
	sphere.albedo[1] = 0.129;
	sphere.albedo[2] = 0.667;
	

	m_scene.addSphere(floorSphere);
	m_scene.addSphere(sphere);
}

void Editor::onUpdate(unsigned int const& viewportTexture, const float& deltaTime)
{
	m_deltaTime = deltaTime;
	newFrame();
	renderSceneGraph();
	renderSettings();
	renderScene(viewportTexture);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void Editor::renderSceneGraph()
{
	ImGui::Begin("SceneGraph");
	if (ImGui::Button("Add Sphere")) {
		Scene::Sphere newSphere;
		m_scene.addSphere(newSphere);
	}

	for (size_t i = 0; i < m_scene.getSpheres().size(); i++)
	{
		Scene::Sphere& sphere = m_scene.getSpheres()[i];
		std::string treeNodeLabel = "Entity " + std::to_string(i);

		if (ImGui::TreeNode("Entity"))
		{
			std::string posLabel = "Position##" + std::to_string(i);
			std::string colorLabel = "Color##" + std::to_string(i);
			std::string radiusLabel = "Radius##" + std::to_string(i);
			std::string roughnessLabel = "Roughness##" + std::to_string(i);
			std::string metalicLabel = "Metalic##" + std::to_string(i);
			std::string emissiveLabel = "EmissivePower##" + std::to_string(i);
			std::string emissiveLabel2 = "EmissiveColor##" + std::to_string(i);
			std::string iorLabel = "ior##" + std::to_string(i);
			std::string deleteLabel = "Delete Sphere##" + std::to_string(i);

			ImGui::DragFloat3(posLabel.c_str(), sphere.position, 0.1, -100.0, 100.0);
            ImGui::ColorEdit3(colorLabel.c_str(), sphere.albedo);
            ImGui::SliderFloat(radiusLabel.c_str(), &sphere.radius, -100.0, 100.0);
			ImGui::DragFloat(roughnessLabel.c_str(), &sphere.roughness, 0.01, 0.0, 1.0);
			ImGui::DragFloat(metalicLabel.c_str(), &sphere.metalic, 0.01, 0.0, 1.0);
			ImGui::DragFloat(emissiveLabel.c_str(), &sphere.emissivePower, 0.01, 0.0, 5.0);
			ImGui::ColorEdit3(emissiveLabel2.c_str(), sphere.emissiveColor);
			//ImGui::DragFloat(iorLabel.c_str(), &sphere.ior, 0.01, 0.0, 5.0);

			if (ImGui::Button(deleteLabel.c_str()))
			{
				auto& spheres = m_scene.getSpheres();
				spheres.erase(spheres.begin() + i);
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}
Scene& Editor::getScene()
{
	return m_scene;
}

void Editor::renderSettings()
{
	ImGui::Begin("Settings");
	auto& settings = m_scene.getRenderSettings();

	ImGui::Text("Render time: %.3f ms", m_deltaTime * 1000);
	if (ImGui::CollapsingHeader("Settings"))
	{
		ImGui::SliderFloat3("Light Direction", &settings.lightDir[0], - 10.0f, 10.0f, "%.6f");
		ImGui::ColorEdit3("BackgroundColor", &settings.backgroundColor[0]);
	}
	ImGui::End();
}

void Editor::renderScene(unsigned int const& viewportTexture)
{
	ImGui::Begin("Scene");
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	float contentRegionAspectRatio = viewportSize.x / viewportSize.y;
	float aspectRatio = static_cast<float>(m_screen->SCR_WIDTH) / static_cast<float>(m_screen->SCR_HEIGHT);
	
	if (contentRegionAspectRatio > aspectRatio)
	{
		float imageWidth = viewportSize.y * aspectRatio;
		float xPadding = (viewportSize.x - imageWidth) / 2;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPadding);
		ImGui::Image((ImTextureID)(intptr_t)viewportTexture, ImVec2(imageWidth, viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
	}

	else
	{
		float imageHeight = viewportSize.x / aspectRatio;
		float yPadding = (viewportSize.y - imageHeight) / 2;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yPadding);
		ImGui::Image((ImTextureID)(intptr_t)viewportTexture, ImVec2(viewportSize.x, imageHeight), ImVec2(0, 1), ImVec2(1, 0));
	}

	ImGui::End();
}

void Editor::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, m_windowFlags);
	ImGui::PopStyleVar(3);
	ImGui::DockSpace(ImGui::GetID("DockSpace"));
	ImGui::End();

}