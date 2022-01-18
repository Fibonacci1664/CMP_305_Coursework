#pragma once
#include "BaseMesh.h"
class CylinderMesh : public BaseMesh
{
public:
	CylinderMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int stacks = 1, int slices = 6, float height = 1.0f, float bottomRadius = 0.1f, float topRadius = 0.05f);
	~CylinderMesh();
	XMMATRIX m_Transform;

protected:
	void initBuffers(ID3D11Device* device);
	
	void init(ID3D11Device* device, int stacks, int slices, float height, float bottomRadius, float topRadius);
};