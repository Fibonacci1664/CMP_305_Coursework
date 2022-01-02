// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include <memory>
#include "Terrain.h"
#include "TerrainShader.h"
#include "CylinderMesh.h"
#include "Leaf.h"
#include "LSystem.h"
#include <stack>
#include "LeafShader.h"
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
	void renderLSystem();
	void gui();

private:
	// Init functions
	void loadTextures();
	void initSceneObjects();
	void initShaders(HWND& hwnd);
	void initLights();
	void initDirLight();
	void initCam();
	void initGUIVars();

	// Terrain Options
	void updateTerrain();
	void checkFaulting();
	void checkParticleDepo();
	void checkPerlinNoise();

	// L-System
	void buildLSystem();
	void buildCyl3DTree(char letter, XMVECTOR& pos, XMVECTOR& dir, XMVECTOR& up, XMVECTOR& fwd, XMVECTOR& left, XMMATRIX& currRot);
	void addCylinder(XMVECTOR& pos, XMMATRIX& currRot, XMVECTOR branchLen, float btmRadius, float topRadius);
	void addLeaf(XMVECTOR& pos, XMMATRIX& currRot);
	void resetLSystem();

	// Render functions
	void buildAllGuiOptions();
	void buildCompleteTerrain();
	void buildSmoothingGui();
	void buildLSystemGUI();
	void buildFaultingGui();
	void buildParticleDepoGui();
	void buildPerlinNoiseGui();
	void renderTerrain();

	// Terrain objects
	Terrain* terrainMesh;
	TerrainShader* terrainShader;
	Light* dirLight;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// GUI bools
	// For Faulting
	bool loopFaulting;
	bool runFaultingIterations;

	// For Particle Depo
	bool loopParticleDepo;
	bool runParticleDepoIterations;

	// For Perlin Noise and fBm
	bool newRandomNoise;
	bool addFixedNoise;
	bool ridgedPerlinToggle;
	bool terracedPerlinToggle;
	bool fBmToggle;
	bool runSingleOctave;
	bool runAllOctaves;

	int terrainResolution;
	int faultingIetrations;
	int particleDepoIterations;
	int fBmOctaves;

	// L-system
	CylinderMesh* m_Cylinder;
	Leaf* quadLeaf;
	LSystem l_System;
	LeafShader* leafShader;
	LightShader* lightShader;

	std::vector<CylinderMesh*> m_CylinderList;
	std::vector<Leaf*> leafList;	
	std::map<std::string, bool> systems;
	std::stack<XMVECTOR> position;
	std::stack<XMMATRIX> rotation;
	std::stack<float> branchLength;
	std::stack<float> topRadStk;
	std::stack<float> btmRadStk;

	int iterations = 0;

	float branchLengthMult = 1.0f;
	float topRad = 0.05f;
	float btmRad = 0.1f;

	bool build3DCylTreeToggle;

	// GUI vals
	float perlinFreq;
	float perlinScale;
	float amplitude;
	float noiseStyleValue;
};

#endif