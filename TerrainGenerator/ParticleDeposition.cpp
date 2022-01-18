/*
 * This is the Particle Deposition class it handles:
 *		- Executing the main algorithm for the particle deposition feature
 *
 * Original @author D. Green.
 *
 * © D. Green. 2022.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#include "ParticleDeposition.h"
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTOR / DESTRUCTOR
ParticleDeposition::ParticleDeposition(int& res, float* heightmp) : resolution(res), heightmap(heightmp)
{
	getNewStartPos = true;
	xPos = 0;
	zPos = 0;
}

ParticleDeposition::~ParticleDeposition()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS
void ParticleDeposition::runParticleDepo()
{
	startParticleDepo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ParticleDeposition::updateHeightMap(float* newHeightMap)
{
	heightmap = newHeightMap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ParticleDeposition::startParticleDepo()
{
	//int randHeight = rand() % 2 + 1;
	float randHeight = (float)rand() / RAND_MAX;
	int oldHeight = heightmap[(zPos * resolution) + xPos];

	// We enter this is the random walk hit the edge of the map
	if (getNewStartPos)
	{
		getNewStartPos = false;
		xPos = rand() % resolution;
		zPos = rand() % resolution;
	}

	//heightmap[(zPos * resolution) + xPos] = heightmap[(zPos * resolution) + xPos] + randHeight;

	// Randomise whether we add or remove height
	/*if (rand() % 2 == 0)
	{
		randHeight = -randHeight;
	}*/
	
	int heightMapIndex = (zPos * resolution) + xPos;

	// Update the surrounding 8 points AND the drop point itself
	// Boundary checks carried out

	/*
	 *		 (xPos - 1, zPos - 1)		(xPos, zPos - 1)		(xPos + 1, zPos - 1)
	 *							 *-------------*---------------*							      z
	 *							/			  /				  /									  ^
	 *						   /			 /				 /									 /
	 *						  /				/				/									/
	 *						 /			   /(xPos, zPos)   /								   /
	 *		(xPos - 1, zPos)*-------------*---------------*(xPos + 1, zPos)					  ---------> x
	 *					   /			 /				 /
	 *					  /				/				/
	 *					 /			   /			   /
	 *					/			  /				  /
	 *				   *-------------*---------------*
	 *(xPos - 1, zPos + 1)	 (xPos, zPos + 1)		  (xPos + 1, zPos + 1)
	 * 
	*/

	heightmap[heightMapIndex] = heightmap[heightMapIndex] + randHeight;

	if (xPos - 1 >= 0)
	{
		heightmap[heightMapIndex - 1] = heightmap[heightMapIndex - 1] + randHeight;
	}
	if (xPos + 1 < (resolution - 1))
	{
		heightmap[heightMapIndex + 1] = heightmap[heightMapIndex + 1] + randHeight;
	}
	if (zPos + 1 < (resolution - 1))
	{
		heightmap[heightMapIndex + resolution] = heightmap[heightMapIndex + resolution] + randHeight;
	}
	if (zPos - 1 >= 0)
	{
		heightmap[heightMapIndex - resolution] = heightmap[heightMapIndex - resolution] + randHeight;
	}
	if (zPos + 1 < (resolution - 1) && xPos + 1 < (resolution - 1))
	{
		heightmap[heightMapIndex + resolution + 1] = heightmap[heightMapIndex + resolution + 1] + randHeight;
	}
	if (zPos + 1 < (resolution - 1) && xPos - 1 >= 0)
	{
		heightmap[heightMapIndex + resolution - 1] = heightmap[heightMapIndex + resolution - 1] + randHeight;
	}
	if (zPos - 1 >= 0 && xPos + 1 < (resolution - 1))
	{
		heightmap[heightMapIndex - resolution + 1] = heightmap[heightMapIndex - resolution + 1] + randHeight;
	}
	if (zPos - 1 >= 0 && xPos - 1 >= 0)
	{
		heightmap[heightMapIndex - resolution - 1] = heightmap[heightMapIndex - resolution - 1] + randHeight;
	}

	// Move some random direction, boundary checks carried out

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
				// Move left 1 vertex
				--xPos;
				// Get the height of this new vertex
				int newHeight = heightmap[(zPos * resolution) + xPos];

				// Keep moving left until we get to a vertex of >= to current vertex height
				// Once we do this will be the vertex we add height to next iteration, then repeat algo
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////