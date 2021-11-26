#pragma once

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

