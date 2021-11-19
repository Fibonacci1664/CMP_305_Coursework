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
	void initGUIToggles();
	void loadTextures();
	void initTerrain();
	void initLightShader(HWND& hwnd);
	void initLights();
	void initDirLight();
	void initCam();
	void initGUIVars();

	// Terrain Options
	void updateTerrain();
	void checkFaulting();
	void checkParticleDepo();

	// Render functions
	void buildAllGuiOptions();
	void buildFaultingGui();
	void buildParticleDepoGui();
	void renderTerrain();

	// Terrain objects
	Terrain* terrainMesh;
	LightShader* lightShader;
	Light* dirLight;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// GUI bools
	bool loopFaulting;
	bool runFaultingIterations;
	bool loopParticleDepo;
	bool runParticleDepoIterations;

	// GUI vals
	int faultingIetrations;
	int particleDepoIterations;

};

#endif