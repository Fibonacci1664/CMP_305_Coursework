#pragma once
#include "PlaneMesh.h"

class Coord;

class TerrainMesh :	public PlaneMesh
{
public:
	TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128);
	~TerrainMesh();

	void Resize( int newResolution );
	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext );

	// Custom functions for terrain gen
	void faulting();
	void particleDeposition();
	void smoothTerrain();
	float genWhiteNoise();
	double genOldPerlinNoise(float xPos, float zPos);
	double genImprovedPerlinNoise(float xPos, float yPos, float zPos);
	double getNoiseType(float xPos, float yPos, float zPos);
	void fracBrownMotion();
	void calcTrigWave(float& height, int& x, int& z, const float& scale);

	const inline int GetResolution(){ return resolution; }

	// GETTERS / SETTERS
	float getAmplitude();
	int getOctaves();
	float getSinPeriodMod();
	float getCosPeriodMod();
	float getPhaseShift();
	float getVerticalShift();
	double getPerlinFreq();
	double getPerlinScale();

	void setAmplitude(float newAmplitude);
	void setSinPeriodMod(float newSinPeriodMOd);
	void setCosPeriodMod(float newCosPeriodMod);
	void setPhaseShift(float newPhaseShift);
	void setVerticalShift(float newVerticalShift);

	void setRebuildHeightMap(bool rebuild);
	void setSmoothHeightMap(bool smooth);
	void setFaultingMap(bool fault);
	void setDepositingParticles(bool depositParticles);
	void setWhiteNoise(bool value);
	void setOldPerlinNoise(bool value);
	void setImprovedPerlinNoise(bool value);
	void setOctaves(int numOfOctaves);
	void setPerlinFreq(float freq);
	void setPerlinScale(float scale);
	void setStepfBm(bool value);
	void setIsTrigWave(bool value);
	void setIsWhiteNoise(bool value);

private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );
	void BuildHeightMap();
	Coord* getPosition(int prevEdge = 0);

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 500.0f;		//What is the width and height of our terrain
	float* heightMap;

	// Wave variables
	float amplitude;
	float sinPeriod;
	float sinPeriodMod;
	float cosPeriodMod;
	float cosPeriod;
	float phaseShift;
	float verticalShift;

	double perlinFreq;
	double perlinScale;

	bool rebuildHeightMap;
	bool smoothHeightMap;
	bool faultingMap;
	bool depositingParticles;
	bool whiteNoise;
	bool oldPerlinNoise;
	bool improvedPerlinNoise;
	bool getNewStartPos;
	bool stepfBm;
	bool isTrigWave;
	bool isWhiteNoise;

	int octaves;
	int xPos;
	int zPos;
};