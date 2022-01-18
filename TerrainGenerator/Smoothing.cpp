/*
 * This is a custom smoothing class it handles:
 *		- Running a Von Neumann neighbourhood smoothing algorithm
 *
 * Original @author D. Green.
 *
 * © D. Green. 2022.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#include "Smoothing.h"

// CONSTRUCTOR / DESTRUCTOR
Smoothing::Smoothing(int& res, float* heightmp, const int& terrainSize) : resolution(res), heightmap(heightmp), terrainSz(terrainSize)
{

}

Smoothing::~Smoothing()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS
void Smoothing::smoothTerrain()
{
	// Create a temp height map data structure to hold the new avg values.
	// We need this so that we do NOT change any of the values in the original height map
	// while we are still operating on it to calculate the avgerages for each index pos
	// Once all avg values have been calculated and this temp has been populated
	// Copy the data from this structure back into the original, overwriting the original data
	float* tempHeightMap = new float[resolution * resolution];

	// Scale everything so that the look is consistent across terrain resolutions
	//const float scale = terrainSz / (float)resolution;

	for (int z = 0; z < resolution; ++z)
	{
		for (int x = 0; x < resolution; ++x)
		{
			float windowTotal = 0.0f;
			int sectionsTotalled = 0;

			// Check left, ensure no LEFT checks are carried out until our 'x' index pos is >= 1 or we will go OOB
			if (x >= 1)
			{
				windowTotal += heightmap[(z * resolution) + (x - 1)];
				++sectionsTotalled;
			}

			// Check up, ensure no UP checks are carried out until our 'y' index pos is >= 1 or we will go OOB
			if (z >= 1)
			{
				windowTotal += heightmap[((z - 1) * resolution) + x];
				++sectionsTotalled;
			}

			// Check right, ensure no RIGHT checks are carried out if our 'x' index pos is == resolution or we will go OOB
			if (x < (resolution - 1))
			{
				windowTotal += heightmap[(z * resolution) + (x + 1)];
				++sectionsTotalled;
			}

			// Check down, ensure no DOWN checks are carried out if our 'y' index pos is == resolution or we will go OOB
			if (z < (resolution - 1))
			{
				windowTotal += heightmap[((z + 1) * resolution) + x];
				++sectionsTotalled;
			}

			// Set new smooth data.
			tempHeightMap[(z * resolution) + x] = (heightmap[(z * resolution) + x] + (windowTotal / sectionsTotalled)) * 0.5f;
		}
	}

	// Once tempHeightMap is populated with the avg data values
	// Copy this data back into the original heightMap structure
	for (int z = 0; z < resolution; ++z)
	{
		for (int x = 0; x < resolution; ++x)
		{
			heightmap[(z * resolution) + x] = tempHeightMap[(z * resolution) + x];
		}
	}

	// Then clean up temp
	delete[] tempHeightMap;
	tempHeightMap = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////