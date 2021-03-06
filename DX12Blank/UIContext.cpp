
#include "stdafx.h"
#include "UIContext.h"
#include "MathHelper.h"
#include "CreviceWindow.h"
#include "imgui.h"

int UIContext::Scene = 0;

bool UIContext::Wireframe = false;
bool UIContext::DebugGrid = true;
bool UIContext::UseRayTracing = false;

int UIContext::HDRSkybox = 1;
int UIContext::PBRModel = 0;
bool UIContext::ShowPBRModelCombo = true;

float UIContext::Time = 12.0f;
float UIContext::LightIntensity = 1.0f;
float3 UIContext::LightColor = float3(1, 1, 1);

int UIContext::CubemapRotation = 0;

int UIContext::FOV = 60;
float UIContext::Near = 0.1f;
float UIContext::Far = 800.0f;

bool UIContext::EnableSSAO = true;
float UIContext::OcclusionRadius = 0.26f;
float UIContext::OcclusionPower = 8.0f;
float UIContext::OcclusionFalloff = -0.4f;
float UIContext::OcclusionDarkness = 1.0f;
float UIContext::OcclusionRangeCheck = 1.0f;
int UIContext::OcclusionSamplesCount = 1;

int UIContext::VariableRateShading = 0;

static bool showSceneSettings = true;
static bool showCameraSettings = false;
static bool showPostProcessSettings = false;

bool UIContext::IsRayTracingSupported = false;
bool UIContext::IsVRSSupported = false;

void UIContext::DrawUI(CreviceWindow* window)
{
	bool addPlane = false, addBox = false, addSphere = false, addCone = false, addCylinder = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Save")) {}
			if (ImGui::MenuItem("Save as")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit")) {}
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
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Plane")) { addPlane = true; }
			if (ImGui::MenuItem("Box")) { addBox = true; }
			if (ImGui::MenuItem("Sphere")) { addSphere = true; }
			if (ImGui::MenuItem("Cone")) { addCone = true; }
			if (ImGui::MenuItem("Cylinder")) { addCylinder = true; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Scene Settings", NULL, &showSceneSettings)) {}
			if (ImGui::MenuItem("Camera Settings", NULL, &showCameraSettings)) {}
			if (ImGui::MenuItem("Post Process Settings", NULL, &showPostProcessSettings)) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ShowAddObjectSettings(window, addPlane, addBox, addSphere, addCone, addCylinder);
	if (showSceneSettings) ShowSceneSettings();
	if (showCameraSettings) ShowCameraSettings();
	if (showPostProcessSettings) ShowPostProcessSettings(window);
}

void UIContext::ShowAddObjectSettings(CreviceWindow* window, bool addPlane, bool addBox, bool addSphere, bool addCone, bool addCylinder)
{
	static float f[3]; 
	static int	 i[3];

	if (addPlane)
	{
		f[0] = 4.0f, f[1] = 4.0f; i[0] = 4, i[1] = 4;
		ImGui::OpenPopup("Add plane settings");
	}
		
	if (ImGui::BeginPopupModal("Add plane settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Plane");
		ImGui::Separator();
		ImGui::DragFloat("Width", &f[0], 0.01f);
		ImGui::DragFloat("Depth", &f[1], 0.01f);
		ImGui::DragInt("Slices X", &i[0]);
		ImGui::DragInt("Slices Y", &i[1]);

		if (ImGui::Button("OK", ImVec2(120, 0))) 
		{ 
			window->AddObject(eObject::Plane, f[0], f[1], (float)i[0], (float)i[1]);
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (addBox)
	{
		f[0] = 1.0f, f[1] = 1.0f, f[2] = 1.0f; i[0] = 1;
		ImGui::OpenPopup("Add box settings");
	}

	if (ImGui::BeginPopupModal("Add box settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Box");
		ImGui::Separator();

		ImGui::DragFloat("Width", &f[0], 0.01f);
		ImGui::DragFloat("Height", &f[1], 0.01f);
		ImGui::DragFloat("Depth", &f[2], 0.01f);
		ImGui::DragInt("Subdivisions", &i[0]);

		if (ImGui::Button("OK", ImVec2(120, 0))) 
		{ 
			window->AddObject(eObject::Box, f[0], f[1], f[2], (float)i[0]);
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (addSphere)
	{
		f[0] = 1.0f; i[0] = 16, i[1] = 16;
		ImGui::OpenPopup("Add sphere settings");
	}

	if (ImGui::BeginPopupModal("Add sphere settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Sphere");
		ImGui::Separator();

		ImGui::DragFloat("Radius", &f[0], 0.01f);
		ImGui::DragInt("Slices", &i[0]);
		ImGui::DragInt("Stacks", &i[1]);

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			window->AddObject(eObject::Sphere, f[0], (float)i[0], (float)i[1]);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (addCone)
	{
		f[0] = 1.0f, f[1] = 3.0f; i[0] = 8, i[1] = 8;
		ImGui::OpenPopup("Add cone settings");
	}

	if (ImGui::BeginPopupModal("Add cone settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Cone");
	
		ImGui::DragFloat("Radius", &f[0], 0.01f);
		ImGui::DragFloat("Height", &f[1], 0.01f);
		ImGui::DragInt("Slices", &i[0]);
		ImGui::DragInt("Stacks", &i[1]);

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			window->AddObject(eObject::Cone, f[0], f[1], f[2], (float)i[0], (float)i[1]);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (addCylinder)
	{
		f[0] = 1.0f, f[1] = 3.0f; i[0] = 8, i[1] = 8;
		ImGui::OpenPopup("Add cylinder settings");
	}

	if (ImGui::BeginPopupModal("Add cylinder settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Cylinder");
		ImGui::Separator();

		ImGui::DragFloat("Radius", &f[0], 0.01f);
		ImGui::DragFloat("Height", &f[1], 0.01f);
		ImGui::DragInt("Slices", &i[0]);
		ImGui::DragInt("Stacks", &i[1]);

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			window->AddObject(eObject::Cylinder, f[0], f[1], (float)i[0], (float)i[1]);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void UIContext::ShowSceneSettings()
{
	ImGui::Begin("Scene Settings", &showSceneSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		const char* scene[] = { "PBR Model", "Simple Solids" };
		ImGui::Combo("Scene", &Scene, scene, IM_ARRAYSIZE(scene));

		ImGui::Checkbox("Wireframe", &Wireframe);
		ImGui::Checkbox("Grid", &DebugGrid);
		if (IsRayTracingSupported)
		{
			ImGui::Checkbox("RayTracing", &UseRayTracing);
		}
		
		if (IsVRSSupported)
		{
			const char* vrs[] = { "1x1", "1x2", "2x1", "2x2", "2x4", "4x2", "4x4" };
			ImGui::Combo("Variable Shading Rate", &VariableRateShading, vrs, IM_ARRAYSIZE(vrs));
		}

		ImGui::Separator();

		const char* skyboxes[] = { "Black", "Street", "Rooftop", "Cape Hill", "Venice Sunset", "Newport Loft" };
		ImGui::Combo("HDR Skybox", &HDRSkybox, skyboxes, IM_ARRAYSIZE(skyboxes));

		ImGui::Separator();

		ImGui::SliderFloat("Time", &Time, 0.0f, 24.0f);
		ImGui::SliderFloat("Light Intensity", &LightIntensity, 0.0f, 100.0f);
		ImGui::ColorEdit3("Light Color", &LightColor.x);

		ImGui::Separator();

		ImGui::SliderInt("Skybox Rotation", &CubemapRotation, 0, 360);

		ImGui::Separator();

		if (ShowPBRModelCombo)
		{
			const char* models[] = { "Sphere", "Cerberus" };
			ImGui::Combo("PBR Models", &PBRModel, models, IM_ARRAYSIZE(models));
		}
	}
	ImGui::End();
}

void UIContext::ShowCameraSettings()
{
	ImGui::Begin("Camera Settings", &showCameraSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		ImGui::SliderInt("FOV", &FOV, 30, 90);
		ImGui::SliderFloat("Near", &Near, 0.01f, 1.0f);
		ImGui::SliderFloat("Far", &Far, 100.0f, 4000.0f);
	}
	ImGui::End();
}


void UIContext::ShowPostProcessSettings(CreviceWindow* window)
{
	ImGui::Begin("Post Process Settings", &showPostProcessSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		ImGui::Checkbox("Enable SSAO", &EnableSSAO);
		ImGui::SliderFloat("AO Radius", &OcclusionRadius, 0.01f, 2.0f);
		ImGui::SliderFloat("AO Strength", &OcclusionPower, 1.0f, 20.0f);
		if (window->GetDevice().UseRayTracing())
		{
			ImGui::SliderInt("AO Samples Count", &OcclusionSamplesCount, 1, 4);
		}
		else
		{
			ImGui::SliderFloat("AO Falloff", &OcclusionFalloff, -1.0f, 1.0f);
			ImGui::SliderFloat("AO Darkness", &OcclusionDarkness, 1.0f, 10.0f);
			ImGui::SliderFloat("AO Range", &OcclusionRangeCheck, 0.0f, 10.0f);
		}
	}
	ImGui::End();
}

void UIContext::ShowObjectSettings(RenderObject& ro)
{
	ImGui::Begin("Object settings", &showCameraSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	{
		ImGui::Text("Transform");
		ImGui::DragFloat3("Position", &ro.m_transform.m_translation.x, 0.01f);
		ImGui::DragFloat3("Rotation", &ro.m_transform.m_rotation.x, 0.01f);
		ImGui::DragFloat3("Scale", &ro.m_transform.m_scale.x, 0.01f);

		ImGui::Text("Material");
		ImGui::ColorEdit3("Color", &ro.m_color.x);
		ImGui::SliderFloat("Roughness", &ro.m_roughness, 0.0f, 1.0f);
		ImGui::SliderFloat("Metalness", &ro.m_metalness, 0.0f, 1.0f);
	}
	ImGui::End();
}
