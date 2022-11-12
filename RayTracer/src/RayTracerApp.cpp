#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "Walnut/Random.h"

#include "Renderer.h"
#include "Camera.h"

#include <memory>
#include "glm/gtc/type_ptr.hpp"

#include "Trolling.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : m_Camera(45.0f, 0.1f, 100.0f), m_imgWidth(0), m_imgHeight(0)
	{
		m_Scene.Spheres.push_back(Sphere{ {0.0f, 0.0f, 0.0f}, 0.5f, {1.0f, 0.0f, 1.0f} });
		m_Scene.Spheres.push_back(Sphere{ {1.0f, 0.0f, -5.0f}, 1.5f, {0.1f, 0.8f, 0.3f} });
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		if (!m_Rendering)
		{
			if (ImGui::Button("Render"))
				m_Rendering = !m_Rendering;
		}
		else if (ImGui::Button("Stop"))
				m_Rendering = !m_Rendering;
		ImGui::Checkbox("Multithreaded", &m_Multithreaded);
		ImGui::SliderInt("No. threads", &m_NrThreads, 2, 16);
		ImGui::Text("Last render time: %.3fms", m_LastRenderTime);
		ImGui::End();

		ImGui::Begin("Scene");

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.001f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.001f);
			ImGui::ColorEdit3("Color", glm::value_ptr(sphere.Albedo));
			ImGui::Separator();

			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		m_imgWidth = ImGui::GetContentRegionAvail().x;
		m_imgHeight = ImGui::GetContentRegionAvail().y;

		if (auto image = m_Renderer.GetFinalImage())
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		if (!m_Rendering)
		{
			m_Renderer.Clear();
			return;
		}

		Timer timer;

		m_Renderer.OnResize(m_imgWidth, m_imgHeight);
		m_Camera.OnResize(m_imgWidth, m_imgHeight);

		if (!m_Multithreaded)
			m_Renderer.Render(m_Scene, m_Camera);
		else
			m_Renderer.RenderMT(m_Scene, m_Camera, m_NrThreads);

		m_LastRenderTime = timer.ElapsedMillis();
	}	

private:
	Scene m_Scene;
	Renderer m_Renderer;
	Camera m_Camera;
	int m_imgWidth, m_imgHeight;

	bool m_Rendering = true;
	bool m_Multithreaded = false;
	int m_NrThreads = 4;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::MenuItem("Crash"))
				CrashProgram();
			if (ImGui::MenuItem("Exit"))
				app->Close();
			ImGui::EndMenu();
		}
	});
	return app;
}