#pragma once
#include <d3d11.h>

struct Particle
{
	// Construct a particle at the passed in pos(x, y)
	Particle(XMFLOAT3 pos)
	{
		position = pos;
	}

	/*int x;
	int y;*/

	XMFLOAT3 position = { 0, 0, 0 };
	XMFLOAT3 speed = { 0, 0, 0 };

	//float speed[2] = { 0, 0 };

	float volume = 1.0f;		// The total volume of the particle, a new particle starts at max volume
	float sediment = 0.0f;		// The fractional amount of the total volume represented as sediment, a new particle starts at zero
};