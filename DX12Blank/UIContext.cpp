
#include "stdafx.h"
#include "UIContext.h"
#include "imgui.h"

bool UIContext::Wireframe = false;

int UIContext::HDRSkybox = 0;
int UIContext::PBRModel = 0;

float UIContext::Color[3] = { 1.0f, 1.0f, 1.0f };
float UIContext::Roughness = 0.5f;
float UIContext::Metalness = 0.5f;

void UIContext::DrawUI()
{
	bool isOpen = true;
	ImGui::Begin("View settings", &isOpen);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		ImGui::Checkbox("Wireframe", &Wireframe);

		const char* skyboxes[] = { "Street" };
		ImGui::Combo("HDR Skybox", &HDRSkybox, skyboxes, IM_ARRAYSIZE(skyboxes));

		const char* models[] = { "Sphere", "Cerberus" };
		ImGui::Combo("PBR Models", &PBRModel, models, IM_ARRAYSIZE(models));

		if (PBRModel == 0)
		{
			ImGui::ColorEdit3("Color", Color);
			ImGui::SliderFloat("Roughness", &Roughness, 0.0f, 1.0f);
			ImGui::SliderFloat("Metalness", &Metalness, 0.0f, 1.0f);
		}
	}
	ImGui::End();
}

