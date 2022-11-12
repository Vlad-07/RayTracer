#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Walnut/Image.h"
#include "Scene.h"
#include "Camera.h"
#include "Ray.h"

#include <thread>

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Render(const Scene& scene, const Camera& camera);
	void RenderMT(const Scene& scene, const Camera& camera, int threads); // currently slower than the single-thread func because the renderer is faster than the thread management required

	void Clear();
	void OnResize(int width, int height);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }
	uint32_t* GetFinalImageData() { return m_ImageData; }


private:
	struct HitData
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectId;
	};

public:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

private:
	HitData TraceRay(const Ray& ray);
	HitData ClosestHit(const Ray& ray, float hitDistance, int objectId);
	HitData Miss(const Ray& ray);

public:
	static Renderer* s_Instance;

private:
	uint32_t* m_ImageData = nullptr;
	std::shared_ptr<Walnut::Image> m_FinalImage;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	glm::vec3 m_LightDir = { -1, -1, -1 };


	std::thread* tPool;
};