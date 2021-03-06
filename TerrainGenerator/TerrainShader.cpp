/*
 * This is the Terrain Shader class it handles:
 *		- Init the Light buffer
 *		- Init the matrix buffer
 *		- Init the texture sampler
 *		- Init a texture bounds buffer used for blending textures
 *		- Init a noise type buffer used to determine which type of noise is being generated
 *
 *
 * Original @author Abertay University.
 * Updated by @author D. Green.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#include "TerrainShader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTOR / DESTRUCTOR
TerrainShader::TerrainShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"terrain_vs.cso", L"terrain_ps.cso");
}

TerrainShader::~TerrainShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	
	if (noiseStyleBuffer)
	{
		noiseStyleBuffer->Release();
		noiseStyleBuffer = 0;
	}
	
	if (texturingBoundsBuffer)
	{
		texturingBoundsBuffer->Release();
		texturingBoundsBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS
void TerrainShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC nosieStyleBufferDesc;
	D3D11_BUFFER_DESC texturingBoundsBufferDesc;

	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
	
	nosieStyleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	nosieStyleBufferDesc.ByteWidth = sizeof(NoiseBufferType);
	nosieStyleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	nosieStyleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	nosieStyleBufferDesc.MiscFlags = 0;
	nosieStyleBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &noiseStyleBuffer);
	
	texturingBoundsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	texturingBoundsBufferDesc.ByteWidth = sizeof(TextureBoundsBufferType);
	texturingBoundsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	texturingBoundsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texturingBoundsBufferDesc.MiscFlags = 0;
	texturingBoundsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &texturingBoundsBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TerrainShader::setShaderParameters(ID3D11DeviceContext* deviceContext,
	const XMMATRIX &worldMatrix,
	const XMMATRIX &viewMatrix,
	const XMMATRIX &projectionMatrix,
	ID3D11ShaderResourceView* texture1,
	ID3D11ShaderResourceView* texture2,
	ID3D11ShaderResourceView* texture3,
	Light* light,
	float style,
	XMFLOAT4 normalTexturingBounds,
	XMFLOAT4 ridgedTexturingBounds)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->direction = light->getDirection();
	lightPtr->padding = 0.0f;
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	NoiseBufferType* noiseStylePtr;
	deviceContext->Map(noiseStyleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	noiseStylePtr = (NoiseBufferType*)mappedResource.pData;
	noiseStylePtr->noiseStyle = style;
	noiseStylePtr->noisePadding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(noiseStyleBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &noiseStyleBuffer);

	TextureBoundsBufferType* texturingBoundsPtr;
	deviceContext->Map(texturingBoundsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	texturingBoundsPtr = (TextureBoundsBufferType*)mappedResource.pData;
	texturingBoundsPtr->N_waterLowerBound = normalTexturingBounds.x;
	texturingBoundsPtr->N_waterUpperbound = normalTexturingBounds.y;
	texturingBoundsPtr->N_grassLowerBound = normalTexturingBounds.z;
	texturingBoundsPtr->N_grassUpperBound = normalTexturingBounds.w;

	texturingBoundsPtr->R_waterLowerBound = ridgedTexturingBounds.x;
	texturingBoundsPtr->R_waterUpperbound = ridgedTexturingBounds.y;
	texturingBoundsPtr->R_grassLowerBound = ridgedTexturingBounds.z;
	texturingBoundsPtr->R_grassUpperBound = ridgedTexturingBounds.w;
	deviceContext->Unmap(texturingBoundsBuffer, 0);
	deviceContext->PSSetConstantBuffers(2, 1, &texturingBoundsBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture1);
	deviceContext->PSSetShaderResources(1, 1, &texture2);
	deviceContext->PSSetShaderResources(2, 1, &texture3);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////