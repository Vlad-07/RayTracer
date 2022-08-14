#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "Walnut/Random.h"

#include "Renderer.h"

#include <memory>

#include "Trolling.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
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
		ImGui::Text("Last render time: %.3fms", m_LastRenderTime);
		ImGui::NewLine();
		ImGui::ColorPicker3("Sphere color", (float*)&renderer.m_SphereCol);
		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		m_imgWidth = ImGui::GetContentRegionAvail().x;
		m_imgHeight = ImGui::GetContentRegionAvail().y;

		if (auto image = renderer.GetFinalImage())
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		if (!m_Rendering)
		{
			renderer.Clear();
			return;
		}

		Timer timer;

		renderer.OnResize(m_imgWidth, m_imgHeight);
		renderer.RenderMT(2);

		m_LastRenderTime = timer.ElapsedMillis();
	}	

private:
	Renderer renderer;
	bool m_Rendering = true;
	int m_imgWidth, m_imgHeight;
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