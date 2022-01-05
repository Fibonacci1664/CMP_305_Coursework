#pragma once
#include <string>

#include "PlaneMesh.h"
#include "Faulting.h"
#include "ParticleDeposition.h"
#include "PerlinNoise.h"
#include "Smoothing.h"
#include <array>
#include <vector>

struct HeightAndGradient
{
	float height;
	float gradientX;
	float gradientZ;
};

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

	// Stuff for hydraulic erosion
	
	VertexType* vertices;

	// Erosion Steps
	bool active = false;
	int remaining = 200000;
	int erosionstep = 1000;

	// Particle Properties
	//float dt = 1.2;
	float density = 1.0;			// This provides varying amounts of inertia
	float evapRate = 0.001;
	float depositionRate = 0.1;
	float minVol = 0.01;
	float friction = 0.05;


	// Seb L
	int erosionRadius = 3;
	float inertia = 0.8f; // At zero, water will instantly change direction to flow downhill. At 1, water will never change direction. 
	float sedimentCapacity = 3.0f; // Multiplier for how much sediment a droplet can carry
	float minSedimentCapacity = 0.01f; // Used to prevent carry capacity getting too close to zero on flatter terrain
	float erodeSpeed = 5.0f;
	float depositSpeed = 0.1f;
	float evaporateSpeed = 0.03f;
	float gravity = 4.0f;
	int maxDropletLifetime = 35;	// This ensures we do not get 'immortal' particles roaming around
	//float initialWaterVolume = 1.0f;		// This part of the particle
	float initialSpeed = 1.0f;

	std::vector<std::vector<int>> erosionBrushIndicesVec;
	std::vector<std::vector<float>> erosionBrushWeightsVec;
};