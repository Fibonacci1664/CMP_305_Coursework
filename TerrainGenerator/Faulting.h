#pragma once
//#include "Line.h"
#include <DirectXMath.h>

using namespace DirectX;

struct Coord
{
	int x;
	int y;
	int z;
	int edge = 0;
};

struct Line
{
	XMFLOAT3 startPos;
	XMFLOAT3 endPos;
};

class Faulting
{
public:
	Faulting(int& res, float* heightmp);
	~Faulting();

	void createFault();
	void updateHeightMap(float* newHeightMap);

private:
	
	Coord* getPosition(int prevEdge = 0);

	int& resolution;
	float* heightmap;
};