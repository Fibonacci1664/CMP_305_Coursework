/*
 * This is a custom Perlin Noise class it handles:
 *		- Calling specific versions of Perlin Noise:
 *			* Old (Classic) Perlin Noise algorithm
 *			* Improved PErlin Noise algorithm
 *		- Modifying the type of noise return to create varieties of noise:
 *			* Ridged noise
 *			* Terraced noise
 *
 * Original @author D. Green.
 *
 * © D. Green. 2022.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#pragma once
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PerlinNoise
{
public:
	PerlinNoise(int& res, float* heightmp, const int& terrainSize);
	~PerlinNoise();

	void updateHeightMap(float* newHeightMap);

	void buildPerlinNoise();
	void fracBrownianMotion();
	void setFrequency(double freq);
	void setScale(double scl);
	void setAmplitude(float amp);
	void setRidged(bool isRidged);
	void setTerraced(bool isTerraced);
	void setPerlinAlgorithm(char type);
	float getFreq();
	float getAmplitude();

private:
	double genOldPerlinNoise(float xPos, float zPos);
	double genImprovedPerlinNoise(float xPos, float yPos, float zPos);
	
	bool ridgedPerlin;
	bool terracedPerlin;
	bool oldPerlin;
	bool improvedPerlin;

	int& resolution;
	const int& terrainSz;

	float* heightmap;
	float amplitude;

	double perlinFreq;
	double perlinScale;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////