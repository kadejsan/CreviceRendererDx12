#pragma once

#include "RenderObject.h"

class CreviceWindow;

class UIContext
{
public:
	static void DrawUI(CreviceWindow* window);

	static void ShowAddObjectSettings(CreviceWindow* window, bool addPlane, bool addBox, bool addSphere, bool addCone, bool addCylinder);
	static void ShowSceneSettings();
	static void ShowCameraSettings();
	static void ShowPostProcessSettings();
	static void ShowObjectSettings(RenderObject& ro);

	static bool Wireframe;
	static bool DebugGrid;
	static bool UseRayTracing;

	static int HDRSkybox;
	static int PBRModel;

	static float Time;
	static float LightIntensity;
	static float3 LightColor;

	static int CubemapRotation;

	static int FOV;
	static float Near;
	static float Far;

	static bool  EnableSSAO;
	static float OcclusionRadius;
	static float OcclusionPower;
	static float OcclusionFalloff;
	static float OcclusionDarkness;
	static float OcclusionRangeCheck;
};