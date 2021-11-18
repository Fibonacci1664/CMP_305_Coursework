// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include <memory>
#include "Terrain.h"
#include "LightShader.h"

using std::unique_ptr;

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	// Init functions
	void loadTextures();
	void initTerrain();
	void initLightShader(HWND& hwnd);
	void initLights();
	void initDirLight();
	void initCam();

	// Render functions
	void renderTerrain();

	// Terrain objects
	Terrain* terrainMesh;
	LightShader* lightShader;
	Light* dirLight;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

};

#endif