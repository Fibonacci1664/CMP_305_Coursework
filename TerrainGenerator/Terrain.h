#pragma once
#include <string>

#include "PlaneMesh.h"
#include "Faulting.h"
#include "ParticleDeposition.h"
#include "PerlinNoise.h"

class Terrain : public PlaneMesh
{
public:
	Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128);
	~Terrain();

	void regenerateTerrain();
	void resetTerrain();
	void resize(int& newResolution);
	void updateHeightMap();

	// Generate terrain effects
	void generateFault();
	void startParticleDepo();
	void genPerlinNoise();
	void generatefBm();

	// Getters and Setters
	//const inline int getResolution() { return resolution; }
	int getTerrainRes();
	void setPNFreqScaleAmp(float freq, float scale, float amplitude);
	void setPerlinRidged(bool isRidged);
	void setPerlinAlgoType(char type);
	void setStepfBm(bool isStep);
	void setfBmOctaves(int numOfOctaves);
	float getPerlinFreq();
	float getPerlinAmplitude();

private:
	void initTerrain(int& newResolution, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void initTerrainObjects();	
	void generateTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void buildTerrain();
	void createBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices);
	
	const float uvScale = 10.0f;			// Tile the UV map 10 times across the plane
	const float terrainSize = 500.0f;		// What is the width and height of our terrain
	float* heightMap;

	bool newTerrain = false;
	bool isFaulting = false;

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

	// Terrain Features
	Faulting* faulting;
	ParticleDeposition* particleDepo;
	PerlinNoise* perlinNoise;
};