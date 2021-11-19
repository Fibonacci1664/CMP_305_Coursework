#include "ParticleDeposition.h"
#include <cstdlib>

ParticleDeposition::ParticleDeposition(int& res, float* heightmp) : resolution(res), heightmap(heightmp)
{
	getNewStartPos = false;
	xPos = 0;
	zPos = 0;
}

ParticleDeposition::~ParticleDeposition()
{

}

void ParticleDeposition::runParticleDepo()
{
	startParticleDepo();
}

void ParticleDeposition::updateHeightMap(float* newHeightMap)
{
	heightmap = newHeightMap;
}

void ParticleDeposition::startParticleDepo()
{
	int randHeight = rand() % 2 + 1;
	int oldHeight = heightmap[(zPos * resolution) + xPos];

	if (getNewStartPos)
	{
		getNewStartPos = false;
		xPos = rand() % resolution;
		zPos = rand() % resolution;
	}

	heightmap[(zPos * resolution) + xPos] = heightmap[(zPos * resolution) + xPos] + randHeight;

	/*
	* LEFT	=	1
	* RIGHT	=	2
	* UP	=	3
	* DOWN	=	4
	*/
	int randDir = 0;
	randDir = rand() % 4 + 1;

	switch (randDir)
	{
		case 1:	// MOVE LEFT
		{
			// Boundary check
			if (xPos >= 1)
			{
				--xPos;
				int newHeight = heightmap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && xPos >= 1)
				{
					--xPos;
					oldHeight = newHeight;
					newHeight = heightmap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		case 2:	// MOVE RIGHT
		{
			// Boundary check
			if (xPos < (resolution - 1))
			{
				++xPos;
				int newHeight = heightmap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && xPos < (resolution - 1))
				{
					++xPos;
					oldHeight = newHeight;
					newHeight = heightmap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		case 3:	// MOVE UP
		{
			// Boundary check
			if (zPos >= 1)
			{
				--zPos;
				int newHeight = heightmap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && zPos >= 1)
				{
					--zPos;
					oldHeight = newHeight;
					newHeight = heightmap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		case 4:	// MOVE DOWN
		{
			// Boundary check
			if (zPos < (resolution - 1))
			{
				++zPos;
				int newHeight = heightmap[(zPos * resolution) + xPos];

				while ((newHeight < oldHeight) && zPos < (resolution - 1))
				{
					++zPos;
					oldHeight = newHeight;
					newHeight = heightmap[(zPos * resolution) + xPos];
				}

				break;
			}
			else
			{
				getNewStartPos = true;
				break;
			}
		}
		default:
		{
			// Some failure message here!
		}
	}
}
