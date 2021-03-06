/*
 * This is the Line and Coord struct and is used in conjunction
 * with the particle deposition algorithm.
 *
 * Updated by @author D. Green.
 * 
 * ? D. Green. 2021.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Coord
{
	int x;
	int y;
	int z;
	int edge = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Line
{
	XMFLOAT3 startPos;
	XMFLOAT3 endPos;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////