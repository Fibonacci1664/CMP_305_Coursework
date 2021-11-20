#include "PerlinNoise.h"
#include "OldPerlinNoise.h"
#include "ImprovedPerlin.h"

PerlinNoise::PerlinNoise(int& res, float* heightmp, const int& terrainSize) : resolution(res), heightmap(heightmp), terrainSz(terrainSize)
{
	// Default values
	ridgedPerlin = false;
	oldPerlin = false;
	improvedPerlin = false;

	perlinFreq = 0.0f;
	perlinScale = 0.0f;
	amplitude = 0.0f;
}

PerlinNoise::~PerlinNoise()
{

}

void PerlinNoise::updateHeightMap(float* newHeightMap)
{
	heightmap = newHeightMap;
}

double PerlinNoise::genOldPerlinNoise(float xPos, float zPos)
{
	// Same values in, same terrain out
	float vec[2] = { xPos * perlinScale * perlinFreq, zPos * perlinScale * perlinFreq };
	//float vec[2] = { xPos * perlinScale + perlinFreq, zPos * perlinScale + perlinFreq };
	//float vec[2] = { z, x };		// Test to ensure that without the rand scale value, perlin noise returned zero, and it does!
	double perlinNoise = OldPerlinNoise::noise2D(vec);

	return perlinNoise;
}

double PerlinNoise::genImprovedPerlinNoise(float xPos, float yPos, float zPos)
{
	double vec[3] = { xPos * perlinScale * perlinFreq, yPos, zPos * perlinScale * perlinFreq };
	//double vec[3] = { xPos * perlinScale + perlinFreq, yPos, zPos * perlinScale + perlinFreq };
	double improvedPerlin = ImprovedPerlin::noise(vec[0], vec[1], vec[2]);

	return improvedPerlin;
}

void PerlinNoise::fracBrownianMotion()
{
	buildPerlinNoise();
	amplitude *= 0.5;
	perlinFreq *= 2;
}

void PerlinNoise::buildPerlinNoise()
{
	float height = 0.0f;
	double noise = 0.0f;

	// Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSz / (float)resolution;

	for (int z = 0; z < (resolution); z++)
	{
		for (int x = 0; x < (resolution); x++)
		{
			// What noise are we using?
			if (oldPerlin)
			{
				noise = genOldPerlinNoise(x, z);
			}
			else if (improvedPerlin)
			{
				noise = genImprovedPerlinNoise(x, 0, z);
			}

			// Is it going to be ridged or not?
			if (ridgedPerlin)
			{
				height = abs(amplitude * noise);
				height *= -1.0f;
			}
			else
			{
				height = amplitude * noise;
			}
			
			// Set the height
			heightmap[(z * resolution) + x] += height;
		}
	}
}

void PerlinNoise::setFrequency(double freq)
{
	perlinFreq = freq;
}

void PerlinNoise::setScale(double scl)
{
	perlinScale = scl;
}

void PerlinNoise::setAmplitude(float amp)
{
	amplitude = amp;
}

void PerlinNoise::setRidged(bool isRidged)
{
	ridgedPerlin = isRidged;
}

void PerlinNoise::setPerlinAlgorithm(char type)
{
	if (type == 'O')
	{
		oldPerlin = true;
		improvedPerlin = false;
	}
	else if (type == 'I')
	{
		improvedPerlin = true;
		oldPerlin = false;
	}
}

float PerlinNoise::getFreq()
{
	return perlinFreq;
}

float PerlinNoise::getAmplitude()
{
	return amplitude;
}