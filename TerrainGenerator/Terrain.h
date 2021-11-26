#pragma once
#include <string>

#include "PlaneMesh.h"
#include "Faulting.h"
#include "ParticleDeposition.h"
#include "PerlinNoise.h"
#include "Smoothing.h"

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

	// Getters and Setters
	int getTerrainRes();
	void setPNFreqScaleAmp(float freq, float scale, float amplitude);
	void setPerlinRidged(bool isRidged);
	void setPerlinTerraced(bool isTerraced);
	void setPerlinAlgoType(char type);
	float getPerlinFreq();
	float getPerlinAmplitude();

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
};