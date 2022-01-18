/*
 * This is the main application file which constructs the entire scene, it handles:
 *		- Initialisation of all textures
 *		- Initialisation of all scene objects
 *		- Initialisation of all shaders
 *		- Initialisation of all lights
 *
 *		- Processing GUI input to render various terrain features
 *
 *		- Rendering of the terrain, and L-System
 *		- Rendering and updating the GUI
 *
 * Original @author Abertay University.
 * Updated by @author D. Green.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _APP1_H
#define _APP1_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using std::unique_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	void checkSmoothing();
	void checkParticleDepo();
	void checkPerlinNoise();
	void adjustedTextureBounds();
	void initialTextureBounds();

	// L-System
	void buildLSystem();
	void buildCyl3DTree(char letter, XMVECTOR& pos, XMVECTOR& dir, XMVECTOR& up, XMVECTOR& fwd, XMVECTOR& left, XMMATRIX& currRot);
	void addCylinder(XMVECTOR& pos, XMMATRIX& currRot, XMVECTOR branchLen, float btmRadius, float topRadius);
	void addLeaf(XMVECTOR& pos, XMMATRIX& currRot);
	void resetLSystem();

	// Render functions
	void buildAllGuiOptions();
	void buildHydErosionGui();
	void buildCompleteTerrainGui();
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
	bool buildingFullTerrain;
	bool smoothingValsSet;
	bool fBmValsSet;
	bool haveEroded;

	// For Smoothing
	bool loopSmoothing;
	bool runSmoothingIterations;
	 
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
	int faultingIterations;
	int particleDepoIterations;
	int smoothingIterations;
	int fBmOctaves;

	float N_waterLowerBound;
	float N_waterUpperbound;
	float N_grassLowerBound;
	float N_grassUpperBound;

	float R_waterLowerBound;
	float R_waterUpperbound;
	float R_grassLowerBound;
	float R_grassUpperBound;

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

	// Hydraulic Erosion
	int erosionIterations = 300000;

	int erosionRadius = 3;
	float inertia = 0.5f;					// At zero, water will instantly change direction to flow directly downhill. At 1, water will never change direction. 
	float sedimentCapacity = 1.1f;			// Multiplier for how much sediment a droplet can carry
	float minSedimentCapacity = 0.01f;		// Used to prevent carry capacity getting too close to zero on flatter terrain
	float erodeSpeed = 0.5f;
	float depositSpeed = 0.012f;
	float evaporateSpeed = 0.012f;
	//float gravity = 4.0f;
	int maxDropletLifetime = 30;			// This ensures we do not get 'immortal' particles roaming around
	//float initialWaterVolume = 1.0f;		// This part of the particle
	//float initialSpeed = 1.0f;

	// GUI vals
	float perlinFreq;
	float perlinScale;
	float amplitude;
	float noiseStyleValue;
};

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////