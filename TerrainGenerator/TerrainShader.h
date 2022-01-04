#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class TerrainShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct NoiseBufferType
	{
		float noiseStyle;
		XMFLOAT3 noisePadding;
	};

	struct TextureBoundsBufferType
	{
		// N = for normal height texturing
		float N_sandLowerBound;
		float N_sandUpperbound;
		float N_grassLowerBound;
		float N_grassUpperBound;

		// R = for ridged height texturing
		float R_sandLowerBound;
		float R_sandUpperbound;
		float R_grassLowerBound;
		float R_grassUpperBound;
	};

public:
	TerrainShader(ID3D11Device* device, HWND hwnd);
	~TerrainShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext,
		const XMMATRIX &world,
		const XMMATRIX &view,
		const XMMATRIX &projection,
		ID3D11ShaderResourceView* texture1,
		ID3D11ShaderResourceView* texture2,
		ID3D11ShaderResourceView* texture3,
		Light* light,
		float noiseStyle,
		XMFLOAT4 normalTexturingBounds,
		XMFLOAT4 ridgedTexturingBounds);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer * matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* noiseStyleBuffer;
	ID3D11Buffer* texturingBoundsBuffer;

	ID3D11SamplerState* sampleState;
};
