#pragma once

class UIContext
{
public:
	static void DrawUI();
	
	static void ShowSceneSettings();
	static void ShowCameraSettings();

	static bool Wireframe;
	static bool DebugGrid;

	static int HDRSkybox;
	static int PBRModel;

	static int CubemapRotation;

	static float Color[3];
	static float Roughness;
	static float Metalness;

	static int FOV;
	static float Near;
	static float Far;
};