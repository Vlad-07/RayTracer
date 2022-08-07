#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Walnut/Image.h"

class Renderer
{
public:
	Renderer() = default;

	void Render();
	void Clear();
	void OnResize(int width, int height);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }
	uint32_t* GetFinalImageData() { return m_ImageData; }

private:
	glm::vec4 PerPixel(glm::vec2 coord);

public:
	glm::vec3 m_SphereCol = { 0.2f, 0.8f, 0.2f };
	glm::vec3 m_LightDir = {-1, -1, -1};

private:
	uint32_t* m_ImageData = nullptr;
	std::shared_ptr<Walnut::Image> m_FinalImage;
};