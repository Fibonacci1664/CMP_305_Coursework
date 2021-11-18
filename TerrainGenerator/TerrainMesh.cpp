#include <DirectXMath.h>

#include "OldPerlinNoise.h"
#include "ImprovedPerlin.h"
#include "TerrainMesh.h"
#include "Line.h"

TerrainMesh::TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution ) :
	PlaneMesh( device, deviceContext, lresolution ) 
{
	Resize( resolution );
	rebuildHeightMap = true;
	Regenerate( device, deviceContext );

	amplitude = 0;
	sinPeriod = 0;
	sinPeriodMod = 0.0f;
	cosPeriodMod = 0.0f;
	cosPeriod = 0;
	phaseShift = 0;
	verticalShift = 0;

	smoothHeightMap = false;
	faultingMap = false;
	depositingParticles = false;
	whiteNoise = false;
	oldPerlinNoise = false;
	improvedPerlinNoise = false;
	getNewStartPos = true;
	stepfBm = false;
	isTrigWave = false;
	isWhiteNoise = false;

	xPos = 0;
	zPos = 0;
	octaves = 0;
	amplitude = 0.0;
	perlinFreq = 0.0l;
	perlinScale = 0.0l;
}

//Cleanup the heightMap
TerrainMesh::~TerrainMesh()
{
	delete[] heightMap;
	heightMap = 0;
}

//Fill an array of floats that represent the height values at each grid point.
//Here we are producing a Sine wave along the X-axis
void TerrainMesh::BuildHeightMap()
{
	float height;
	double noise;
	float value;

	// Scale everything so that the look is consistent across terrain resolutions
	const float scale =  terrainSize / (float)resolution;

	for( int z = 0; z < ( resolution ); z++ )
	{
		for( int x = 0; x < ( resolution ); x++ )
		{
			if (isTrigWave)
			{
				calcTrigWave(height, x, z, scale);
				heightMap[(z * resolution) + x] = height;
				continue;
			}

			// Could tidy this up by just having 1 bool to check if noise is true or not
			if (octaves > 0 || stepfBm || isWhiteNoise)
			{
				noise = getNoiseType(x, 0, z);
				height = amplitude * noise;
				heightMap[(z * resolution) + x] += height;
			}
			else
			{
				// This is needed for the first time the height map is built
				// we can't += to an index position that isn't intialised
				// also used for subsequent builds that are NOT fBm
				height = (float)amplitude * sin(sinPeriod + phaseShift) + verticalShift;
				height += (float)amplitude * cos(cosPeriod + phaseShift) + verticalShift;
				heightMap[(z * resolution) + x] = height;
			}

			/*height = amplitude * noise;

			heightMap[(z * resolution) + x] += noise * amplitude;*/
		}
	}	
}

void TerrainMesh::calcTrigWave(float& height, int& x, int& z, const float& scale)
{
	// My mods
	// y = A sin(B(x + C)) + D
	/*
	* Amplitude is A
	* Period is 2PI/B
	* Phase shift is C (Positive to the left)
	* Vertical shift is D
	*/

	sinPeriod = (float)x * sinPeriodMod * scale;
	cosPeriod = (float)z * cosPeriodMod * scale;

	//ORIGNAL SIN WAVE
			//				 A			sin(     B        (x + C) ) +      D
	height = (float)amplitude * sin(sinPeriod + phaseShift) + verticalShift;

	// Compound sin waves
	height += 0.5f * (float)amplitude * sin(2.0f * sinPeriod + phaseShift) + verticalShift;
	height += 0.2f * (float)amplitude * sin(3.0f * sinPeriod + phaseShift) + verticalShift;

	// ORIGINAL COS WAVE
	height += (float)amplitude * cos(cosPeriod + phaseShift) + verticalShift;

	// Compound cos waves
	height += 0.5f * (float)amplitude * cos(2.0f * cosPeriod + phaseShift) + verticalShift;
	height += 0.8f * (float)amplitude * cos(0.5f * cosPeriod + phaseShift) + verticalShift;
}

void TerrainMesh::fracBrownMotion()
{
	if (stepfBm)
	{
		BuildHeightMap();
		amplitude *= 0.5;
		perlinFreq *= 2;
		stepfBm = false;
	}
	else
	{
		for (int i = 0; i < octaves; ++i)
		{
			BuildHeightMap();
			amplitude *= 0.5;
			perlinFreq *= 2;
		}
	}
}

double TerrainMesh::getNoiseType(float xPos, float yPos, float zPos)
{
	double noise = 0.0l;

	if (whiteNoise)
	{
		noise = genWhiteNoise();
		return noise;
	}
	else if (oldPerlinNoise)
	{
		noise = genOldPerlinNoise(xPos, zPos);
		return noise;
	}
	else if (improvedPerlinNoise)
	{
		noise = genImprovedPerlinNoise(xPos, 0, zPos);
		return noise;
	}

	return noise;	
}

Coord* TerrainMesh::getPosition(int prevEdge)
{
	Coord* position = new Coord;

	// Random value between 1 and 4 to represent the "edges" of the plane
	int randEdge = 0;
	bool positionSet = false;

	// Get end coords from one of the other edges, try again if the rand end point
	// is on the same edge as the rand start point
	while (!positionSet)
	{
		// Gen/regen rand edge value
		randEdge = rand() % 4 + 1;

		// Only switch if we are getting a point on a different edge
		if (randEdge != prevEdge)
		{
			// Get end coords from one of the other edges
			switch (randEdge)
			{
				case 1:
				{
					// Left hand "edge" of plane
					position->x = 0;
					position->z = rand() % resolution;
					position->edge = 1;
					break;
				}
				case 2:
				{
					// Right hand "edge" of plane
					position->x = resolution - 1;
					position->z = rand() % resolution;
					position->edge = 2;
					break;
				}
				case 3:
				{
					// Top "edge" of plane
					position->x = rand() % resolution;
					position->z = 0;
					position->edge = 3;
					break;
				}
				case 4:
				{
					// Bottom "edge" of plane
					position->x = rand() % resolution;
					position->z = resolution - 1;
					position->edge = 4;
					break;
				}
				default:
				{
					// Need some tell tale here that this broke!
				}
			}

			positionSet = true;
		}		
	}

	return position;
}

void TerrainMesh::faulting()
{
	// PLAN
	/*
	 Pick a random index where x = 0, and z = randVal
	 Then pick another random index where x = resolution - 1, z = randVal
	 Then create a line between these two points, need to create a line struct.
	 This line is our fault line (L1).
	 Iterate over each point in the plane and calculate a new line (L2) from
	 startPos to each iterated point
	 Then work out the cross product of L1 and L2 and if the result if +y then
	 move that index value up, if the result of the cross product is -y then
	 move that index value down.

	 Repeat each time we fault but make sure that the start point is on a random edge each time
	*/

	// Write faulting code here.
	Line L1;
	Coord* startPos = getPosition();

	L1.startPos.x = startPos->x;
	L1.startPos.y = 0;
	L1.startPos.z = startPos->z;

	Coord* endPos = getPosition(startPos->edge);

	L1.endPos.x = endPos->x;
	L1.endPos.y = 0;
	L1.endPos.z = endPos->z;

	// Create a vector from our new line(L1), Vector 1
	XMFLOAT3 V1;
	V1.x = L1.endPos.x - L1.startPos.x;
	V1.y = L1.endPos.y - L1.startPos.y;
	V1.z = L1.endPos.z - L1.startPos.z;

	// Line 2(L2) start pos will always be the same as the L1 starting position
	Line L2;
	L2.startPos.x = L1.startPos.x;
	L2.startPos.y = 0;
	L2.startPos.z = L1.startPos.z;

	// The vector we'll use and which is calculated during each iteration
	XMFLOAT3 V2;
	XMFLOAT3 returnVec;
	XMVECTOR resultantVec;
	
	// Loop over heightmap, and create a new line (L2) using L1 startPos(x, y)
	// as the startPos, and the current index(x, y) as the endPos.
	for (int z = 0; z < resolution; z++)
	{
		for (int x = 0; x < resolution; x++)
		{
			// Use each individual indice as the endpoint of L2
			L2.endPos.x = x;
			L2.endPos.y = 0;
			L2.endPos.z = z;

			// Create a second vector using the new endpoints, each and every iteration
			V2.x = L2.endPos.x - L2.startPos.x;
			V2.y = L2.endPos.y = L2.startPos.y;
			V2.z = L2.endPos.z - L2.startPos.z;

			// We now have 2 vectors V1 & V2 to carry out a cross product with
			// Take the cross product of vectors V1 & V2, then check the y-value
			// If y = +, increase the value stored at the current index of heightMap
			// If y = -, decrease the value stored at the current index of heightMap

			// Load the float3's into the D3D11 cross prodcut func, which take vecs but we can use the load float3 func
			resultantVec = XMVector3Cross(XMLoadFloat3(&V1), XMLoadFloat3(&V2));

			// Then we need to take the resultant vec from the cross product and shove it back into a float3
			XMStoreFloat3(&returnVec, resultantVec);

			// Then check the y-value of the cross prodcuts resultant float3
			if (returnVec.y > 0)
			{
				// Increment the current index pos value;
				heightMap[(z * resolution) + x] = heightMap[(z * resolution) + x] + 1.0f;
			}
			else
			{
				// Decrement the current index pos value;
				heightMap[(z * resolution) + x] = heightMap[(z * resolution) + x] - 1.0f;
			}
		}
	}
}

// ATTEMPT No.2 hahahah
void TerrainMesh::particleDeposition()
{
	int randHeight = rand() % 2 + 1;
	int oldHeight = heightMap[(zPos * resolution) + xPos];

	if (getNewStartPos)
	{
		getNewStartPos = false;
		xPos = rand() % resolution;
		zPos = rand() % resolution;
	}

	heightMap[(zPos * resolution) + xPos] = heightMap[(zPos * resolution) + xPos] + randHeight;

	/*
	* LEFT	=	1
	* RIGHT	=	2
	* UP	=	3
	* DOWN	=	4
	*/
	int randDir = 0;
	randDir = rand() % 4 + 1;

	switch (randDir)
	{
		case 1:	// MOVE LEFT
		{
			// Boundary check
			if(xPos >= 1)
			{
				--xPos;
				int newHeight = heightMap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && xPos >= 1)
				{
					--xPos;
					oldHeight = newHeight;
					newHeight = heightMap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		case 2:	// MOVE RIGHT
		{
			// Boundary check
			if (xPos < (resolution - 1))
			{
				++xPos;
				int newHeight = heightMap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && xPos < (resolution - 1))
				{
					++xPos;
					oldHeight = newHeight;
					newHeight = heightMap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		case 3:	// MOVE UP
		{
			// Boundary check
			if (zPos >= 1)
			{
				--zPos;
				int newHeight = heightMap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && zPos >= 1)
				{
					--zPos;
					oldHeight = newHeight;
					newHeight = heightMap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		case 4:	// MOVE DOWN
		{
			// Boundary check
			if (zPos < (resolution - 1))
			{
				++zPos;
				int newHeight = heightMap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && zPos < (resolution - 1))
				{
					++zPos;
					oldHeight = newHeight;
					newHeight = heightMap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		default:
		{
			// Some failure message here!
		}
	}
}

void TerrainMesh::smoothTerrain()
{
	// Create a temp height map data structure to hold the new avg values.
	// We need this so that we do NOT change any of the value in the original height map
	// while we are still operating on it to calculate the avgerages for each index pos
	// Once all avg values have been calculated and this temp has been populated
	// Copy the data from this structure back into the original, overwriting the original data
	float* tempHeightMap = new float[resolution * resolution];

	for (int z = 0; z < resolution; ++z)
	{
		for (int x = 0; x < resolution; ++x)
		{
			float windowTotal = 0.0f;
			int sectionsTotalled = 0;

			// Check left, ensure no LEFT checks are carried out until our 'x' index pos is >= 1 or we will go OOB
			if (x >= 1)
			{
				windowTotal += heightMap[(z * resolution) + (x - 1)];
				++sectionsTotalled;
			}

			// Check up, ensure no UP checks are carried out until our 'y' index pos is >= 1 or we will go OOB
			if (z >= 1)
			{
				windowTotal += heightMap[((z - 1) * resolution) + x];
				++sectionsTotalled;
			}

			// Check right, ensure no RIGHT checks are carried out if our 'x' index pos is == resolution or we will go OOB
			if (x < (resolution - 1))
			{
				windowTotal += heightMap[(z * resolution) + (x + 1)];
				++sectionsTotalled;
			}

			// Check down, ensure no DOWN checks are carried out if our 'y' index pos is == resolution or we will go OOB
			if (z < (resolution - 1))
			{
				windowTotal += heightMap[((z + 1) * resolution) + x];
				++sectionsTotalled;
			}

			// Set new smooth data.
			tempHeightMap[(z * resolution) + x] = (heightMap[(z * resolution) + x] + (windowTotal / sectionsTotalled)) * 0.5f;
		}
	}

	// Once tempHeightMap is populated with the avg data values
	// Copy this data back into the original heightMap structure
	for (int z = 0; z < resolution; ++z)
	{
		for (int x = 0; x < resolution; ++x)
		{
			heightMap[(z * resolution) + x] = tempHeightMap[(z * resolution) + x];
		}
	}

	// Then clean up temp
	delete[] tempHeightMap;
	tempHeightMap = nullptr;
}

float TerrainMesh::genWhiteNoise()
{
	float randHeightMultpl = (float)rand() / RAND_MAX;		// White noise
	return randHeightMultpl;
}

double TerrainMesh::genOldPerlinNoise(float xPos, float zPos)
{
	// DON'T USE RANDOM VALUES FOR THE SCALE AND OFFSET! THIS DOESN'T WORK FOR SOME REASON??
	//float randScaleVal = (float)rand() / RAND_MAX;		// A random float 0 - 1
	//float randOffsetVal = (float)rand() / RAND_MAX;		// A random float 0 - 1
	//randScaleVal /= 10.0f;
	//randOffsetVal /= 10.0f;
	//float vec[2] = { z * randScaleVal + randOffsetVal, x * randScaleVal + randOffsetVal };

	// Same values, in same terrain out
	float vec[2] = { xPos * perlinScale * perlinFreq, zPos * perlinScale * perlinFreq };
	//float vec[2] = { xPos * perlinScale + perlinFreq, zPos * perlinScale + perlinFreq };
	//float vec[2] = { z, x };		// Test to ensure that without the rand scale value, perlin noise returned zero, and it does!
	double perlinNoise = OldPerlinNoise::noise2D(vec);

	return perlinNoise;
}

double TerrainMesh::genImprovedPerlinNoise(float xPos, float yPos, float zPos)
{
	double vec[3] = { xPos * perlinScale * perlinFreq, yPos, zPos * perlinScale * perlinFreq };
	//double vec[3] = { xPos * perlinScale + perlinFreq, yPos, zPos * perlinScale + perlinFreq };
	double improvedPerlin = ImprovedPerlin::noise(vec[0], vec[1], vec[2]);

	return improvedPerlin;
}

void TerrainMesh::Resize( int newResolution )
{
	resolution = newResolution;
	heightMap = new float[resolution * resolution];
	if( vertexBuffer != NULL ) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;
}

// Set up the heightmap and create or update the appropriate buffers
void TerrainMesh::Regenerate( ID3D11Device * device, ID3D11DeviceContext * deviceContext )
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	
	// Calculate and store the height values, do this once the first time
	// This will only be rebuilt if explicitly called to do so
	if (rebuildHeightMap)
	{
		BuildHeightMap();
		rebuildHeightMap = false;
	}

	if (smoothHeightMap)
	{
		smoothTerrain();
		smoothHeightMap = false;
	}

	if (faultingMap)
	{
		faulting();
		faultingMap = false;
	}

	if (depositingParticles)
	{
		particleDeposition();
		depositingParticles = false;
	}

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ( ( resolution - 1 ) * ( resolution - 1 ) ) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for( j = 0; j < ( resolution ); j++ ) {
		for( i = 0; i < ( resolution ); i++ ) {
			positionX = (float)i * scale;
			positionZ = (float)( j ) * scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3( positionX, height, positionZ );
			vertices[index].texture = XMFLOAT2( u, v );

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {

			//Build index array
			indices[index] = ( j*resolution ) + i;
			indices[index + 1] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			indices[index + 2] = ( ( j + 1 ) * resolution ) + i;

			indices[index + 3] = ( j * resolution ) + i;
			indices[index + 4] = ( j * resolution ) + ( i + 1 );
			indices[index + 5] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			index += 6;
		}
	}

	//Set up normals
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[( j + 1 ) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab( c.x - a.x, c.y - a.y, c.z - a.z );
			XMFLOAT3 ac( b.x - a.x, b.y - a.y, b.z - a.z );
			
			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = ( cross.x * cross.x ) + ( cross.y * cross.y ) + ( cross.z * cross.z );
			mag = sqrtf( mag );
			cross.x/= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal( 0, 1, 0 );
	for( j = 0; j < resolution; j++ ) {
		for( i = 0; i < resolution; i++ ) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if( ( i - 1 ) >= 0 ) {
				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[j * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[j * resolution + ( i - 1 )].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.z;
					count++;
				}
			}
			//right planes
			if( ( i ) <( resolution - 1 ) ) {

				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + i].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + i].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt( ( smoothedNormal.x * smoothedNormal.x ) + ( smoothedNormal.y * smoothedNormal.y ) + ( smoothedNormal.z * smoothedNormal.z ) );
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if( vertexBuffer == NULL ) {
		CreateBuffers( device, vertices, indices );
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map( vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		//  Update the vertex buffer here.
		memcpy( mappedResource.pData, vertices, sizeof( VertexType ) * vertexCount );
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap( vertexBuffer, 0 );
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

//Create the vertex and index buffers that will be passed along to the graphics card for rendering
//For CMP305, you don't need to worry so much about how or why yet, but notice the Vertex buffer is DYNAMIC here as we are changing the values often
void TerrainMesh::CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices ) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof( VertexType ) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer( &vertexBufferDesc, &vertexData, &vertexBuffer );

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer( &indexBufferDesc, &indexData, &indexBuffer );
}

// ################################ GETTERS / SETTERS ################################

float TerrainMesh::getAmplitude()
{
	return amplitude;
}

int TerrainMesh::getOctaves()
{
	return octaves;
}

float TerrainMesh::getSinPeriodMod()
{
	return sinPeriodMod;
}

float TerrainMesh::getCosPeriodMod()
{
	return cosPeriodMod;
}

float TerrainMesh::getPhaseShift()
{
	return phaseShift;
}

float TerrainMesh::getVerticalShift()
{
	return verticalShift;
}

void TerrainMesh::setAmplitude(float newAmplitude)
{
	amplitude = newAmplitude;
}

void TerrainMesh::setSinPeriodMod(float newSinPeriodMod)
{
	sinPeriodMod = newSinPeriodMod;
}

void TerrainMesh::setCosPeriodMod(float newCosPeriodMod)
{
	cosPeriodMod = newCosPeriodMod;
}

void TerrainMesh::setPhaseShift(float newPhaseShift)
{
	phaseShift = newPhaseShift;
}

void TerrainMesh::setVerticalShift(float newVerticalShift)
{
	verticalShift = newVerticalShift;
}

void TerrainMesh::setRebuildHeightMap(bool rebuild)
{
	rebuildHeightMap = rebuild;
}

void TerrainMesh::setSmoothHeightMap(bool smooth)
{
	smoothHeightMap = smooth;
}

void TerrainMesh::setFaultingMap(bool fault)
{
	faultingMap = fault;
}

void TerrainMesh::setDepositingParticles(bool depositParticles)
{
	depositingParticles = depositParticles;
}

void TerrainMesh::setWhiteNoise(bool value)
{
	whiteNoise = value;
}

void TerrainMesh::setOldPerlinNoise(bool value)
{
	oldPerlinNoise = value;
}

void TerrainMesh::setImprovedPerlinNoise(bool value)
{
	improvedPerlinNoise = value;
}

void TerrainMesh::setOctaves(int numOfOctaves)
{
	octaves = numOfOctaves;
}

void TerrainMesh::setPerlinFreq(float freq)
{
	perlinFreq = freq;
}

void TerrainMesh::setPerlinScale(float scale)
{
	perlinScale = scale;
}

void TerrainMesh::setStepfBm(bool value)
{
	stepfBm = value;
}

double TerrainMesh::getPerlinFreq()
{
	return perlinFreq;
}

double TerrainMesh::getPerlinScale()
{
	return perlinScale;
}

void TerrainMesh::setIsTrigWave(bool value)
{
	isTrigWave = value;
}

void TerrainMesh::setIsWhiteNoise(bool value)
{
	isWhiteNoise = value;
}