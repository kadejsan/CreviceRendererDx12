#pragma once

class UIContext
{
public:
	static void DrawUI();

	static bool Wireframe;

	static int HDRSkybox;
	static int PBRModel;

	static int CubemapRotation;

	static float Color[3];
	static float Roughness;
	static float Metalness;
};