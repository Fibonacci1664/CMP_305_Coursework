#include "Terrain.h"
#include "Particle.h"
#include <algorithm>

//int erosionBrushIndices[][];
//float erosionBrushWeights[][];

Terrain::Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution) :
	PlaneMesh(device, deviceContext, resolution)
{
	initTerrain(resolution, device, deviceContext);
}

Terrain::~Terrain()
{
	delete[] heightMap;
	heightMap = 0;

	if (faulting)
	{
		delete faulting;
		faulting = nullptr;
	}

	if (particleDepo)
	{
		delete particleDepo;
		particleDepo = nullptr;
	}

	if (perlinNoise)
	{
		delete perlinNoise;
		perlinNoise = nullptr;
	}

	if (smoothing)
	{
		delete smoothing;
		smoothing = nullptr;
	}
}

void Terrain::resetTerrain()
{
	newTerrain = true;
}

void Terrain::initTerrain(int& resolution, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	newTerrain = true;

	resize(resolution);
	generateTerrain(device, deviceContext);
	initTerrainObjects();
}

void Terrain::initTerrainObjects()
{
	faulting = new Faulting(resolution, heightMap);
	particleDepo = new ParticleDeposition(resolution, heightMap);
	perlinNoise = new PerlinNoise(resolution, heightMap, terrainSize);
	smoothing = new Smoothing(resolution, heightMap, terrainSize);
}

void Terrain::resize(int& newResolution)
{
	newTerrain = true;

	resolution = newResolution;

	// Clean up old heightMap before reassinging or we'll have a mem leak
	delete[] heightMap;
	heightMap = 0;
	
	heightMap = new float[resolution * resolution];

	if (vertexBuffer != NULL)
	{
		vertexBuffer->Release();
	}

	vertexBuffer = NULL;
}

void Terrain::updateHeightMap()
{
	faulting->updateHeightMap(heightMap);
	particleDepo->updateHeightMap(heightMap);
	perlinNoise->updateHeightMap(heightMap);
}

// Generate all the vertices and indice in our terrain
void Terrain::generateTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	if (newTerrain)
	{
		buildTerrain();
		newTerrain = false;
	}

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ((resolution - 1) * (resolution - 1)) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = uvScale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for (j = 0; j < (resolution); j++)
	{
		for (i = 0; i < (resolution); i++)
		{
			positionX = (float)i * scale;
			positionZ = (float)(j)*scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3(positionX, height, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);

			u += increment;
			index++;
		}

		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;

	for (j = 0; j < (resolution - 1); j++)
	{
		for (i = 0; i < (resolution - 1); i++)
		{

			//Build index array
			indices[index] = (j * resolution) + i;
			indices[index + 1] = ((j + 1) * resolution) + (i + 1);
			indices[index + 2] = ((j + 1) * resolution) + i;

			indices[index + 3] = (j * resolution) + i;
			indices[index + 4] = (j * resolution) + (i + 1);
			indices[index + 5] = ((j + 1) * resolution) + (i + 1);
			index += 6;
		}
	}

	//Set up normals
	for (j = 0; j < (resolution - 1); j++)
	{
		for (i = 0; i < (resolution - 1); i++)
		{
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[(j + 1) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab(c.x - a.x, c.y - a.y, c.z - a.z);
			XMFLOAT3 ac(b.x - a.x, b.y - a.y, b.z - a.z);

			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = (cross.x * cross.x) + (cross.y * cross.y) + (cross.z * cross.z);
			mag = sqrtf(mag);
			cross.x /= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal(0, 1, 0);

	for (j = 0; j < resolution; j++)
	{
		for (i = 0; i < resolution; i++)
		{
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;

			//Left planes
			if ((i - 1) >= 0)
			{
				//Top planes
				if ((j) < (resolution - 1))
				{
					smoothedNormal.x += vertices[j * resolution + (i - 1)].normal.x;
					smoothedNormal.y += vertices[j * resolution + (i - 1)].normal.y;
					smoothedNormal.z += vertices[j * resolution + (i - 1)].normal.z;
					count++;
				}
				//Bottom planes
				if ((j - 1) >= 0)
				{
					smoothedNormal.x += vertices[(j - 1) * resolution + (i - 1)].normal.x;
					smoothedNormal.y += vertices[(j - 1) * resolution + (i - 1)].normal.y;
					smoothedNormal.z += vertices[(j - 1) * resolution + (i - 1)].normal.z;
					count++;
				}
			}

			//right planes
			if ((i) < (resolution - 1))
			{
				//Top planes
				if ((j) < (resolution - 1))
				{
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}

				//Bottom planes
				if ((j - 1) >= 0)
				{
					smoothedNormal.x += vertices[(j - 1) * resolution + i].normal.x;
					smoothedNormal.y += vertices[(j - 1) * resolution + i].normal.y;
					smoothedNormal.z += vertices[(j - 1) * resolution + i].normal.z;
					count++;
				}
			}

			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt((smoothedNormal.x * smoothedNormal.x) + (smoothedNormal.y * smoothedNormal.y) + (smoothedNormal.z * smoothedNormal.z));
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}

	// Create our dyanmic Vertex and Index buffers with the vertex and index data
	if (vertexBuffer == NULL)
	{
		createBuffers(device, vertices, indices);
	}
	else
	{
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//  Update the vertex buffer here.
		memcpy(mappedResource.pData, vertices, sizeof(VertexType) * vertexCount);
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap(vertexBuffer, 0);
	}

	// Release the arrays now that the buffers have been created and loaded.
	/*delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;*/
}

void Terrain::buildTerrain()
{
	// Build a new terrain with 0 height values

	float height = 0.0f;

	// Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	for (int j = 0; j < (resolution); j++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[(j * resolution) + i] = height;
		}
	}
}

// Create the vertex and index buffers that will be passed along to the graphics card for rendering
void Terrain::createBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
}

// ###################### GENERATE TERRAIN EFFECTS ######################

void Terrain::erodeTerrain(float dt, int cycles)
{
	// https://www.firespark.de/resources/downloads/implementation%20of%20a%20methode%20for%20hydraulic%20erosion.pdf
	// TRY AGAIN!

	int currentErosionRadius = 0;

	if (erosionBrushIndicesVec.size() == 0 || currentErosionRadius != erosionRadius)
	{
		initializeBrushIndices(resolution, erosionRadius);
		currentErosionRadius = erosionRadius;
	}

	// Create water droplet at some random point on the map
	for (int iteration = 0; iteration < cycles; iteration++)
	{
		// Spawn New Particle at random location on the map
		int initialRandXPos = rand() % resolution;
		int initialRandZPos = rand() % resolution;
		float posX = initialRandXPos;
		float posZ = initialRandZPos;
		float dirX = 0;
		float dirZ = 0;
		float speed = 1.0f;
		float waterVolume = 1.0f; // This is part of the particle
		float sedimentCarried = 0;

		for (int lifetime = 0; lifetime < maxDropletLifetime; lifetime++)
		{
			int nodeX = (int)posX;
			int nodeZ = (int)posZ;
			int dropletIndex = nodeZ * resolution + nodeX;

			// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
			float cellOffsetX = posX - nodeX;
			float cellOffsetY = posZ - nodeZ;

			// Calculate droplet's height and direction of flow with bilinear interpolation of surrounding heights
			HeightAndGradient heightAndGradient = calculateHeightAndGradient(heightMap, resolution, posX, posZ);

			// Update the droplet's direction and position (move position 1 unit regardless of speed)
			dirX = (dirX * inertia - heightAndGradient.gradientX * (1 - inertia));
			dirZ = (dirZ * inertia - heightAndGradient.gradientZ * (1 - inertia));

			// Normalize direction
			float sum = dirX * dirX + dirZ * dirZ;
			float len = sqrt(sum);

			// If the new direction is below tiny threshold value, i.e. almost flat surface
			if (len <= FLT_EPSILON)
			{
				// Pick random direction
				const float MIN_RAND = 0;
				const float MAX_RAND = XM_2PI;
				const float range = MAX_RAND - MIN_RAND;
				float random = range * ((((float)rand()) / (float)RAND_MAX)) + MIN_RAND;
				dirX = cosf(random);
				dirZ = sinf(random);
				//++randomDirs;
			}

			dirX /= len;
			dirZ /= len;

			/*if (len != 0)
			{
				dirX /= len;
				dirZ /= len;
			}*/

			posX += dirX;
			posZ += dirZ;

			// Stop simulating droplet if it's not moving or has flowed over edge of map
			if ((dirX == 0 && dirZ == 0) || posX < 0 || posX >= resolution - 1 || posZ < 0 || posZ >= resolution - 1)
			{
				break;
			}

			// Find the droplet's new height and calculate the deltaHeight
			float newHeight = calculateHeightAndGradient(heightMap, resolution, posX, posZ).height;
			float deltaHeight = newHeight - heightAndGradient.height;

			// If the change in height is positive, i.e. we moved 'uphill' then we deposit some sediment
			// in the pit the particle just ran through.
			// OR if the particle is carrying more sediment than its capacity, we must deposit some
			// DEPOSITION
			if (deltaHeight > 0 || sedimentCarried > sedimentCapacity)
			{
				float amountToDeposit = 0;

				// If moving uphill (deltaHeight > 0) try fill up to the current height,
				// otherwise deposit a fraction of the excess sediment currently carried
				if (deltaHeight > 0)
				{
					amountToDeposit = fmin(deltaHeight, sedimentCarried);
				}
				else
				{
					amountToDeposit = (sedimentCarried - sedimentCapacity) * depositSpeed;
				}

				sedimentCarried -= amountToDeposit;

				// Add the sediment to the four nodes of the current cell using bilinear interpolation
				// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
				heightMap[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
				heightMap[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				heightMap[dropletIndex + resolution] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				heightMap[dropletIndex + resolution + 1] += amountToDeposit * cellOffsetX * cellOffsetY;
			}
			else		// We moved downhill	-	EROSION
			{
				// So we now need to collect sediment into the particle, thus eroding some from the map
				// c = max(?hdif , pminSlope) · vel · water · pcapacity

				// Calculated new sediment capacity?
				//sedimentCapacity = fmax(-deltaHeight, minSedimentCapacity) * erodeSpeed * waterVolume * sedimentCapacity;

				// Erode a fraction of the droplet's current carry capacity.
				// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
				// The final value of this should NEVER be more than the height diff between old position and new position, i.e deltaHeight
				float amountToErode = 0;

				// As long as we've not gone over capacity, calculate a new amount of erosion
				if (sedimentCarried < sedimentCapacity)
				{
					float resultantSediment = (sedimentCapacity - sedimentCarried) * erodeSpeed;
					amountToErode = fmin(resultantSediment, -deltaHeight);
				}

				// Use erosion brush to erode from all nodes inside the droplet's erosion radius
				for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndicesVec[dropletIndex].size(); brushPointIndex++)
				{
					int nodeIndex = erosionBrushIndicesVec[dropletIndex][brushPointIndex];

					if (nodeIndex > (resolution * resolution))
					{
						continue;
					}

					float weighedErodeAmount = amountToErode * erosionBrushWeightsVec[dropletIndex][brushPointIndex];
					float deltaSediment = (heightMap[nodeIndex] < weighedErodeAmount) ? heightMap[nodeIndex] : weighedErodeAmount;
					heightMap[nodeIndex] -= deltaSediment;
					sedimentCarried += deltaSediment;
				}
			}

			// Update droplet's speed and water content
			speed = sqrt(speed * speed + deltaHeight * deltaHeight * gravity);
			waterVolume *= (1 - evaporateSpeed);
		}
	}
}

HeightAndGradient Terrain::calculateHeightAndGradient(float heightMap[], int mapSize, float posX, float posZ)
{
	int coordX = (int)posX;
	int coordZ = (int)posZ;

	// Calculate Particles offset inside the cell (0,0) = at NW node, (1,1) = at SE node
	float xOffset = posX - coordX;
	float zOffset = posZ - coordZ;

	// ############################# MY BILINEAR INTER #############################

	// Bilinear Interpolation
	// https://x-engineer.org/bilinear-interpolation/
	// https://blogs.sas.com/content/iml/2020/05/18/what-is-bilinear-interpolation.html

	/*
	*			--->
	* (0,1)	NW	 |		NE (1,1)
	*		*----*p1----*
	*		|	 |		|
	*	 --------*p3--------
	*		|	 |		|
	*		|	 |		|
	* (0,0)	*----*p2-----* (1,0)
	*		SW	 |		SE 
	*			--->
	* 
	* 
	* x1 = 0 
	*		--> SW
	* y1 = 0
	* x2 = 1
	*		--> NE
	* y2 = 1
	*/

	// Calculate heights of the four nodes of the Particles cell
	// SW is the (0,0) position
	/*int zeroZeroNode = coordZ * mapSize + coordX;

	float heightSW = heightMap[zeroZeroNode];
	float heightNW = heightMap[zeroZeroNode - mapSize];
	float heightSE = heightMap[zeroZeroNode + 1];
	float heightNE = heightMap[zeroZeroNode - mapSize + 1];*/

	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
	// Formula = R1(x, y) = Q11 · (x2 – x) / (x2 – x1) + Q21 · (x – x1) / (x2 – x1)
	// float southPos = heightSW * (1 - xOffset) / (1 - 0) + heightSE * (xOffset - 0) / (1 - 0);	// Find point p2
	// This can be simplified
	//float southPos = heightSW * (1 - xOffset) + heightSE * xOffset;									// Find point p2

	// Formula = R2(x, y) = Q12 · (x2 – x) / (x2 – x1) + Q22 · (x – x1) / (x2 – x1)
	//float northPos = heightNW * (1 - xOffset) / (1 - 0) + heightNE * (xOffset - 0) / (1 - 0);		// Find point p1
	// Again this can be simplified
	//float northPos = heightNW * (1 - xOffset) + heightNE * xOffset;									// Find point p1

	// Formula = P(x, y) = R1 · (y2 – y) / (y2 – y1) + R2 · (y – y1) / (y2 – y1)
	//float interPos = southPos * (1 - zOffset) / (1 - 0) + northPos * (zOffset - 0) / (1 - 0);		// Find the interpolated point
	// And again this can be simplified
	//float interPos = southPos * (1 - zOffset) + northPos * zOffset;									// Find the interpolated point

	// ############################# MY BILINEAR INTER END #############################

	// Calculate heights of the four nodes of the droplet's cell
	int nodeIndexNW = coordZ * mapSize + coordX;
	float heightNW = heightMap[nodeIndexNW];
	float heightNE = heightMap[nodeIndexNW + 1];
	float heightSW = heightMap[nodeIndexNW + mapSize];
	float heightSE = heightMap[nodeIndexNW + mapSize + 1];

	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
	float gradientX = (heightNE - heightNW) * (1 - zOffset) + (heightSE - heightSW) * zOffset;
	float gradientZ = (heightSW - heightNW) * (1 - xOffset) + (heightSE - heightNE) * xOffset;

	// Calculate height with bilinear interpolation of the heights of the nodes of the cell
	float height = heightNW * (1 - xOffset) * (1 - zOffset) + heightNE * xOffset * (1 - zOffset) + heightSW * (1 - xOffset) * zOffset + heightSE * xOffset * zOffset;

	HeightAndGradient heightAndGrad;
	heightAndGrad.height = height;
	heightAndGrad.gradientX = gradientX;
	heightAndGrad.gradientZ = gradientZ;

	return heightAndGrad;
}

void Terrain::initializeBrushIndices(int mapSize, int radius)
{
	/*erosionBrushIndices = new int[mapSize * mapSize];
	erosionBrushWeights = new float[mapSize * mapSize];*/

	int size = mapSize * mapSize;

	erosionBrushIndicesVec.resize(size);
	erosionBrushWeightsVec.resize(size);

	std::vector<int> xOffsets;
	std::vector<int> zOffsets;
	std::vector<float> weights;

	/*int* xOffsets = new int[radius * radius * 4];
	int* zOffsets = new int[radius * radius * 4];
	float* weights = new float[radius * radius * 4];*/
	float weightSum = 0;
	int addIndex = 0;

	for (int i = 0; i < (mapSize * mapSize); i++)
	{
		int centreX = i % mapSize;
		int centreZ = i / mapSize;

		if (centreZ <= radius || centreZ >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius)
		{
			weightSum = 0;
			addIndex = 0;

			for (int z = -radius; z <= radius; z++)
			{
				for (int x = -radius; x <= radius; x++)
				{
					float sqrDst = x * x + z * z;

					if (sqrDst < radius * radius)
					{
						int coordX = centreX + x;
						int coordY = centreZ + z;

						if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize)
						{
							float weight = 1 - sqrt(sqrDst) / radius;
							weightSum += weight;

							weights.push_back(weight);
							xOffsets.push_back(x);
							zOffsets.push_back(z);

							/*weights[addIndex] = weight;
							xOffsets[addIndex] = x;
							zOffsets[addIndex] = z;*/

							addIndex++;
						}
					}
				}
			}
		}

		int numEntries = addIndex;

		/*int* erosionBrushIndicesArr = new int[numEntries];
		float* erosionBrushWeightsArr = new float[numEntries];*/

		std::vector<int> tempBrushVec;
		std::vector<float> tempWeightsVec;

		for (int j = 0; j < numEntries; j++)
		{
			/*erosionBrushIndicesArr[j] = (zOffsets[j] + centreZ) * mapSize + xOffsets[j] + centreX;
			erosionBrushWeightsArr[j] = weights[j] / weightSum;*/
			
			tempBrushVec.push_back((zOffsets[j] + centreZ) * mapSize + xOffsets[j] + centreX);
			tempWeightsVec.push_back(weights[j] / weightSum);
		}

		erosionBrushIndicesVec[i] = tempBrushVec;
		erosionBrushWeightsVec[i] = tempWeightsVec;
	}
}

void Terrain::generateFault()
{
	faulting->createFault();
}

void Terrain::startParticleDepo()
{
	particleDepo->runParticleDepo();
}

void Terrain::genPerlinNoise()
{
	perlinNoise->buildPerlinNoise();
}

void Terrain::generatefBm()
{
	perlinNoise->fracBrownianMotion();
}

void Terrain::smoothTerrain()
{
	smoothing->smoothTerrain();
}

int Terrain::getTerrainRes()
{
	return resolution;
}

void Terrain::setPNFreqScaleAmp(float freq, float scale, float amplitude)
{
	perlinNoise->setFrequency((double)freq);
	perlinNoise->setScale((double)scale);
	perlinNoise->setAmplitude(amplitude);
}

void Terrain::setPerlinRidged(bool isRidged)
{
	perlinNoise->setRidged(isRidged);
}

void Terrain::setPerlinTerraced(bool isTerraced)
{
	perlinNoise->setTerraced(isTerraced);
}

void Terrain::setPerlinAlgoType(char type)
{
	perlinNoise->setPerlinAlgorithm(type);
}

float Terrain::getPerlinFreq()
{
	return perlinNoise->getFreq();
}

float Terrain::getPerlinAmplitude()
{
	return perlinNoise->getAmplitude();
}

void Terrain::setErosionRad(int newRad)
{
	erosionRadius = newRad;
}

void Terrain::setInertia(float newInertia)
{
	inertia = newInertia;
}

void Terrain::setSedimentCap(float newCapacity)
{
	sedimentCapacity = newCapacity;
}

void Terrain::setErosionSpeed(float newErosionSpeed)
{
	erodeSpeed = newErosionSpeed;
}

void Terrain::setDepositSpeed(float newDepositSpeed)
{
	depositSpeed = newDepositSpeed;
}

void Terrain::setEvapSpeed(float newEvapSpeed)
{
	evaporateSpeed = newEvapSpeed;
}

void Terrain::setGravity(float newGravity)
{
	gravity = newGravity;
}

void Terrain::setMaxParticleLifetime(int newLifetime)
{
	maxDropletLifetime = newLifetime;
}