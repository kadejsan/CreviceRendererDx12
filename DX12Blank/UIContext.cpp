
#include "stdafx.h"
#include "UIContext.h"
#include "MathHelper.h"
#include "imgui.h"

bool UIContext::Wireframe = false;

int UIContext::HDRSkybox = 0;
int UIContext::PBRModel = 0;

int UIContext::CubemapRotation = 0;

float UIContext::Color[3] = { 1.0f, 1.0f, 1.0f };
float UIContext::Roughness = 0.5f;
float UIContext::Metalness = 0.5f;

int UIContext::FOV = 60;
float UIContext::Near = 1.0f;
float UIContext::Far = 1000.0f;

static bool showSceneSettings = true;
static bool showCameraSettings = false;

void UIContext::DrawUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Scene Settings", NULL, &showSceneSettings)) {}
			if (ImGui::MenuItem("Camera Settings", NULL, &showCameraSettings)) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (showSceneSettings) ShowSceneSettings();
	if (showCameraSettings) ShowCameraSettings();
}

void UIContext::ShowSceneSettings()
{
	ImGui::Begin("Scene settings", &showSceneSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		ImGui::Checkbox("Wireframe", &Wireframe);

		const char* skyboxes[] = { "Street", "Rooftop", "Cape Hill", "Venice Sunset", "Newport Loft" };
		ImGui::Combo("HDR Skybox", &HDRSkybox, skyboxes, IM_ARRAYSIZE(skyboxes));

		ImGui::SliderInt("Skybox Rotation", &CubemapRotation, 0, 360);

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

void UIContext::ShowCameraSettings()
{
	ImGui::Begin("Camera settings", &showCameraSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		ImGui::SliderInt("FOV", &FOV, 30, 90);
		ImGui::SliderFloat("Near", &Near, 0.01f, 1.0f);
		ImGui::SliderFloat("Far", &Far, 100.0f, 4000.0f);
	}
	ImGui::End();
}