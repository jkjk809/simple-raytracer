#pragma once

#include <vector>
#include "Graphics/RenderSettings.h"

class Scene
{
public:
	struct Sphere
	{
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float padding = 0.0f;
		float radius = 0.5f;
		float roughness = 0.0f;
		float metalic = 0.0f;
		float emissivePower = 0.0f;
		float emissiveColor[3] = { 0.0f, 0.0f, 0.0f };
		float padding2 = 0.0f;
		float albedo[3] = {0.3f, 0.3f, 0.3f};
		float padding4 = 0.0f;
		//float ior = 0.0f;
		//float padding3 = 0.0f;
	};

public:
	
	void addSphere(const Sphere& sphere);
	std::vector<Sphere>& getSpheres();
	RenderSettings& getRenderSettings();

private:
	std::vector<Sphere> m_spheres;
	RenderSettings renderSettings;
};