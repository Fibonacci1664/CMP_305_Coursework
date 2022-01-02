#pragma once

// Leaf Mesh
// Simple square mesh, with texture coordinates and normals.

#include "BaseMesh.h"

using namespace DirectX;

class Leaf : public BaseMesh
{

public:
	Leaf(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMVECTOR position, float scale);
	~Leaf();

	XMMATRIX m_transform;
	XMVECTOR m_position;
	float scaleFactor;

protected:
	void initBuffers(ID3D11Device* device);
};