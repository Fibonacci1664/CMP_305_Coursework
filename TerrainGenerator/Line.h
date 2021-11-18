#pragma once

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