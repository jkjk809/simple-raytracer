#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

class Texture
{
	public:
		Texture();
		Texture(std::string dir, std::string path);

		void generate();
		void load(bool flip = true);
		void loadCubemap(const std::vector<std::string>& faces);
		
		

		void bind(const unsigned int unit);

		unsigned int id;
		std::string dir;
		std::string path;
};

#endif