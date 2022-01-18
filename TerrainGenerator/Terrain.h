/*
 * This is the terrain class, it handles:
 *		- Generating the terrain mesh
 *		- Regenerating the terrain mesh after modifications
 *		- Setting up the terrain mesh buffers
 *		- Passing information from the App class to respective terrain features classes
 *		- Running the Hydraulic Erosion algorithm
 *
 * Original @author Abertay University.
 * Updated by @author D. Green.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#pragma once
#include <string>
#include "PlaneMesh.h"
#include "Faulting.h"
#include "ParticleDeposition.h"
#include "PerlinNoise.h"
#include "Smoothing.h"
#include <array>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Used in the Hydraulic Erosion algorithm
struct HeightAndGradient
{
	float height;
	float gradientX;
	float gradientZ;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Terrain : public PlaneMesh
{
public:
	Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128);
	~Terrain();

	void generateTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void resetTerrain();
	void resize(int& newResolution);
	void updateHeightMap();

	// Generate terrain effects
	void generateFault();
	void startParticleDepo();
	void genPerlinNoise();
	void generatefBm();
	void smoothTerrain();
	void erodeTerrain(int cycles);                 //Perform n erosion cycles
	HeightAndGradient calculateHeightAndGradient(float heightMap[], int mapSize, float posX, float posZ);
	void initializeBrushIndices(int mapSize, int radius);

	// Getters and Setters
	int getTerrainRes();
	void setPNFreqScaleAmp(float freq, float scale, float amplitude);
	void setPerlinRidged(bool isRidged);
	void setPerlinTerraced(bool isTerraced);
	void setPerlinAlgoType(char type);
	float getPerlinFreq();
	float getPerlinAmplitude();

	void setErosionRad(int newRad);
	void setInertia(float newInertia);
	void setSedimentCap(float newCapacity);
	void setErosionSpeed(float newErosionSpeed);
	void setDepositSpeed(float newDepositSpeed);
	void setEvapSpeed(float newEvapSpeed);
	void setGravity(float newGravity);
	void setMaxParticleLifetime(int newLifetime);

private:
	void initTerrain(int& newResolution, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void initTerrainObjects();	
	
	void buildTerrain();
	void createBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices);
	
	const float uvScale = 25.0f;			// Tile the UV map 50 times across the plane
	const float terrainSize = 250.0f;		// What is the width and height of our terrain
	float* heightMap;

	bool newTerrain = false;
	bool isFaulting = false;

	// Terrain Features
	Faulting* faulting;
	ParticleDeposition* particleDepo;
	PerlinNoise* perlinNoise;
	Smoothing* smoothing;

	// For Hydr Eros
	int erosionRadius = 3;
	float inertia = 0.5f;					// At zero, water will instantly change direction to flow directly downhill. At 1, water will never change direction. 
	float sedimentCapacity = 1.1f;			// Multiplier for how much sediment a droplet can carry
	float minSedimentCapacity = 0.01f;		// Used to prevent carry capacity getting too close to zero on flatter terrain
	float erodeSpeed = 0.5f;
	float depositSpeed = 0.012f;
	float evaporateSpeed = 0.012f;
	float gravity = 4.0f;
	int maxDropletLifetime = 30;			// This ensures we do not get 'immortal' particles roaming around
	//float initialWaterVolume = 1.0f;		// This part of the particle
	//float initialSpeed = 1.0f;

	std::vector<std::vector<int>> erosionBrushIndicesVec;
	std::vector<std::vector<float>> erosionBrushWeightsVec;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////