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

void Renderer::Render()
{
	Clear();
	for (int y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (int x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;
			glm::vec4 result = PerPixel(coord);
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


glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDir = { coord.x, coord.y, -1.0f };
	float radius = 1.0f;
	glm::vec3 sphereColor = m_SphereCol;

	float a = glm::dot(rayDir, rayDir);
	float b = 2 * glm::dot(rayOrigin, rayDir);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	float discriminator = b * b - 4.0f * a * c;

	if (discriminator < 0.0f)
		return glm::vec4(0, 0, 0, 1);

	float t0 = (-b + glm::sqrt(discriminator)) / (2.0f * a);
	float closestT = (-b - glm::sqrt(discriminator)) / (2.0f * a);

	glm::vec3 hit = rayOrigin + rayDir * closestT;
	glm::vec3 normal = glm::normalize(hit);


	glm::vec3 lightDir = glm::normalize(m_LightDir);

	float d = glm::max(glm::dot(normal, -lightDir), 0.0f); // = cos(ang)

	return glm::vec4(sphereColor * d, 1.0f);
}