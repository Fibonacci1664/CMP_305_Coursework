#include "Faulting.h"
#include <cstdlib>

Faulting::Faulting(int& res, float* heightmp) : resolution(res), heightmap(heightmp)
{

}

Faulting::~Faulting()
{

}

void Faulting::createFault()
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
			// Use each individual index as the endpoint of L2
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

			// Load the float3's into the D3D11 cross product func, which take vecs but we can use the load float3 func
			resultantVec = XMVector3Cross(XMLoadFloat3(&V1), XMLoadFloat3(&V2));

			// Then we need to take the resultant vec from the cross product and shove it back into a float3
			XMStoreFloat3(&returnVec, resultantVec);

			// Then check the y-value of the cross prodcuts resultant float3
			if (returnVec.y > 0)
			{
				// Increment the current index pos value;
				heightmap[(z * resolution) + x] = heightmap[(z * resolution) + x] + 1.0f;
			}
			else
			{
				// Decrement the current index pos value;
				heightmap[(z * resolution) + x] = heightmap[(z * resolution) + x] - 1.0f;
			}
		}
	}
}

void Faulting::updateHeightMap(float* newHeightMap)
{
	heightmap = newHeightMap;
}

Coord* Faulting::getPosition(int prevEdge)
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
					// "Left hand" edge of plane
					position->x = 0;
					position->z = rand() % resolution;
					position->edge = 1;
					break;
				}
				case 2:
				{
					// "Right hand" edge of plane
					position->x = resolution - 1;
					position->z = rand() % resolution;
					position->edge = 2;
					break;
				}
				case 3:
				{
					// "Top" edge of plane
					position->x = rand() % resolution;
					position->z = 0;
					position->edge = 3;
					break;
				}
				case 4:
				{
					// "Bottom" edge of plane
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