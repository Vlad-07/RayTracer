#include "Renderer.h"
#include "Walnut/Random.h"


namespace Utils
{
	uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

Renderer* Renderer::s_Instance = nullptr;

Renderer::Renderer() : tPool(nullptr)
{
	if (s_Instance)
		__debugbreak();
	s_Instance = this;
}
Renderer::~Renderer()
{
	s_Instance = nullptr;
}


void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	for (int y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (int x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 result = PerPixel(x, y);
			result = glm::clamp(result, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[y * m_FinalImage->GetWidth() + x] = Utils::ConvertToRGBA(result);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

void Renderer::Clear()
{
	if (!m_FinalImage)
		return;
	memset(m_ImageData, 0, 4LL * m_FinalImage->GetWidth() * m_FinalImage->GetHeight());
	m_FinalImage->SetData(m_ImageData);
}

void Renderer::OnResize(int width, int height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	int bounces = 2;
	for (int i = 0; i < bounces; i++)
	{
		HitData data = TraceRay(ray);
		if (data.HitDistance < 0.0f)
		{
			glm::vec3 skyColor(0.0f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(m_LightDir);
		float lightIntensity = glm::max(glm::dot(data.WorldNormal, -lightDir), 0.0f); // = cos(ang)
		const Sphere& sphere = m_ActiveScene->Spheres[data.ObjectId];

		color += sphere.Albedo * lightIntensity * multiplier;
		multiplier *= 0.7f;

		ray.Origin = data.WorldPosition + data.WorldNormal * 0.000001f;
		ray.Direction = glm::reflect(ray.Direction, data.WorldNormal);
	}
	
	return glm::vec4(color, 1.0f);
}

Renderer::HitData Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float hitDistance = FLT_MAX;
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f)
			continue;

//		float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (closestT < hitDistance && closestT > 0.0f)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitData Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectId)
{
	HitData data{};
	data.HitDistance = hitDistance;
	data.ObjectId = objectId;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectId];
	glm::vec3 origin = ray.Origin - closestSphere.Position;
	data.WorldPosition = origin + ray.Direction * hitDistance;
	data.WorldNormal = glm::normalize(data.WorldPosition);

	data.WorldPosition += closestSphere.Position;

	return data;
}

Renderer::HitData Renderer::Miss(const Ray& ray)
{
	HitData data{};
	data.HitDistance = -1.0f;
	return data;
}


//---------------------------------------  MT ---------------------------------------\\

void RenderRegionMT(uint32_t startX, uint32_t startY, uint32_t endX, uint32_t endY);

void Renderer::RenderMT(const Scene& scene, const Camera& camera, int threads)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	int threadStep = m_FinalImage->GetHeight() / threads;
	tPool = new std::thread[threads];
	for (uint8_t i = 1; i <= threads; i++)
		tPool[i - 1] = std::thread(&RenderRegionMT, 0, m_FinalImage->GetWidth(), threadStep * (i - 1), threadStep * i);
	for (uint8_t i = 0; i < threads; i++)
		tPool[i].join();
	delete[] tPool;

	m_FinalImage->SetData(m_ImageData);
}

void RenderRegionMT(uint32_t startX, uint32_t endX, uint32_t startY, uint32_t endY)
{
	for (int y = startY; y < endY; y++)
	{
		for (int x = startX; x < endX; x++)
		{
			glm::vec4 result = Renderer::s_Instance->PerPixel(x, y);
			result = glm::clamp(result, glm::vec4(0.0f), glm::vec4(1.0f));
			Renderer::s_Instance->GetFinalImageData()[y * Renderer::s_Instance->GetFinalImage()->GetWidth() + x] = Utils::ConvertToRGBA(result);
		}
	}
}