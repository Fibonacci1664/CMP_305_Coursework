/*
 * This is a custom smoothing class it handles:
 *		- Running a Von Neumann neighbourhood smoothing algorithm
 *
 * Original @author D. Green.
 *
 * ? D. Green. 2022.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Smoothing
{
public:
	Smoothing(int& res, float* heightmp, const int& terrainSize);
	~Smoothing();

	void smoothTerrain();

private:
	int& resolution;
	const int& terrainSz;

	float* heightmap;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////