#include "Scene.h"

void Scene::addSphere(const Sphere& sphere)
{
	m_spheres.push_back(sphere);
}

std::vector<Scene::Sphere>& Scene::getSpheres()
{
	return m_spheres;
}

RenderSettings& Scene::getRenderSettings()
{
	return renderSettings;
}