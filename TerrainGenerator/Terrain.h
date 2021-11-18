#pragma once
#include "PlaneMesh.h"
#include "Faulting.h"

class Terrain : public PlaneMesh
{
public:
	Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128);
	~Terrain();

private:
	void initTerrain(int& newResolution, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void resize(int& newResolution);
	void generateTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void buildTerrain();
	void generateFullTerrain();

	void createBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices);

	const float uvScale = 10.0f;			// Tile the UV map 10 times across the plane
	const float terrainSize = 500.0f;		// What is the width and height of our terrain
	float* heightMap;

	Faulting* faulting;
};