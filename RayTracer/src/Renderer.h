#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Walnut/Image.h"

#include <thread>

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Render();
	void RenderMT(int threads); // currently slower than the single-thread func because the renderer is faster than the thread management required

	void Clear();
	void OnResize(int width, int height);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }
	uint32_t* GetFinalImageData() { return m_ImageData; }

//private:  // explanation in .cpp file
	glm::vec4 PerPixel(glm::vec2 coord);

public:
	static Renderer* s_Instance;

	glm::vec3 m_SphereCol = { 0.2f, 0.8f, 0.2f };
	glm::vec3 m_LightDir = {-1, -1, -1};

private:
	uint32_t* m_ImageData = nullptr;
	std::shared_ptr<Walnut::Image> m_FinalImage;

	std::thread* tPool;
	int threadStep;
};