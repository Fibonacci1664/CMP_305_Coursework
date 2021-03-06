/*
 * This is the Original(Classic) Prelin Noise class it handles:
 *		- Executing the Original Perlin Noise algorithm
 *
 *
 * Original @author Abertay University.
 * Updated by @author D. Green.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#include "OldPerlinNoise.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OldPerlinNoise::start = true;
int	OldPerlinNoise::perm[B + B + 2];
float OldPerlinNoise::grad1D[B + B + 2];
float OldPerlinNoise::grad2D[B + B + 2][2];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double OldPerlinNoise::noise1D(double arg)
{

	//The integer left and right boundaries of the arg
	int bx0 = 0, bx1 = 0;

	//The fractional remainder (arg - the boundary integer)
	float rx0 = 0.f, rx1 = 0.f;

	//the easing value
	float sx = 0.f;

	float u = 0.f, v = 0.f, vec[1]{ arg };

	if (start) {
		start = false;
		init();
	}

	//Initialise all our variables
	setup(vec, 0, bx0, bx1, rx0, rx1);

	//Calculate what point of the blend curve we are at between the two integer boundaries
	sx = s_curve(rx0);

	//p[bx0] gives a random number, so g1[p[bx0]] picks a random gradient
	//u & v are the gradient value at the given offset
	u = rx0 * grad1D[perm[bx0]];
	v = rx1 * grad1D[perm[bx1]];

	//Now we blend the two balues based on our position on the curve
	return lerp(sx, u, v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double OldPerlinNoise::noise2D(float vec[2])
{
	//The integer left & right x values and the bottom & top y values. These define what "cell" we are in
	int bx0 = 0, bx1 = 0, by0 = 0, by1 = 0;

	//How far into the current "cell" we are. This is the fractional part of our point.
	float rx0 = 0.f, rx1 = 0.f, ry0 = 0.f, ry1 = 0.f;

	//We'll store pseudo-random numbers for each corner here, we'll then use that to get a pseudo-random gradient
	int b00 = 0, b10 = 0, b01 = 0, b11 = 0;

	//The easing value along the x and y for our point
	float sx = 0.f, sy = 0.f;

	//Some variables used to store values during calculations
	float* q, a = 0.f, b = 0.f, u = 0.f, v = 0.f;
	int i, j;

	//If we've not done this already, build the permutation and gradient lookup tables
	if (start)
	{
		start = 0;
		init();
	}

	//Initialise all our variables on the X & Y
	setup(vec, 0, bx0, bx1, rx0, rx1);
	setup(vec, 1, by0, by1, ry0, ry1);

	//Get a pseudo-random number for the x boundaries
	i = perm[bx0];
	j = perm[bx1];

	//Use this pseudo-random number to get another pseudo-random number for the four corners
	b00 = perm[i + by0];
	b10 = perm[j + by0];
	b01 = perm[i + by1];
	b11 = perm[j + by1];

	//Work out our position on the blend curve (3t^2 - 2t^3 here)
	sx = s_curve(rx0);
	sy = s_curve(ry0);

	//q is set to the pseudo-random gradient
	//Solve for the bottom two points
	q = grad2D[b00];
	u = dotProduct(rx0, ry0, q[0], q[1]);	//U is the weighting of the gradient based on the distance of the point from this corner (0,0)
	q = grad2D[b10];
	v = dotProduct(rx1, ry0, q[0], q[1]);	//V is the weighting of the gradient based on the distance of the point from this corner (1,0)
	a = lerp(sx, u, v);						//a is the the value of U and V blended together using the horizontal easing curve

	//Solve for the top two points
	q = grad2D[b01];
	u = dotProduct(rx0, ry1, q[0], q[1]);	//U is the weighting of the gradient based on the distance of the point from this corner (0,1)
	q = grad2D[b11];
	v = dotProduct(rx1, ry1, q[0], q[1]);	//V is the weighting of the gradient based on the distance of the point from this corner (1,1)
	b = lerp(sx, u, v);						//b is the the value of U and V blended together using the horizontal easing curve

	//The final value is 'a' and 'b' blended together using the vertical easing curve
	return lerp(sy, a, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function just sets up a table of numbers which we use for generating a pseudo-random number later
//and then generates arrays of pseudo-random gradients that we can access using the pseudo-random numbers
//You don't need to worry too much about this
void OldPerlinNoise::init(void)
{
	int i, j, k;

	for (i = 0; i < B; i++)
	{
		perm[i] = i;

		grad1D[i] = (float)((rand() % (B + B)) - B) / B;

		for (j = 0; j < 2; j++)
		{
			grad2D[i][j] = (float)((rand() % (B + B)) - B) / B;
		}
			
		normalize2D(grad2D[i]);
	}

	while (--i)
	{
		k = perm[i];
		perm[i] = perm[j = rand() % B];
		perm[j] = k;
	}

	for (i = 0; i < B + 2; i++)
	{
		perm[B + i] = perm[i];
		grad1D[B + i] = grad1D[i];

		for (j = 0; j < 2; j++)
		{
			grad2D[B + i][j] = grad2D[i][j];
		}	
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Setup the function variables
//b0 and b1 are the integer values before and after our point
//r0 and r1 are the fractional distances from the integer boundaries
//This is an old school way of doing it, which I've kept in to be as close to the original implementation as possible
void OldPerlinNoise::setup(float* vec, int i, int& b0, int& b1, float& r0, float& r1)
{
	float t = vec[i] + N;
	b0 = ((int)t) & BM;
	b1 = (b0 + 1) & BM;
	r0 = t - (int)t;
	r1 = r0 - 1.;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Function for normalising a 2D vector
const void OldPerlinNoise::normalize2D(float v[2])
{
	float s = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Function for normalising a 3D vector
const void OldPerlinNoise::normalize3D(float v[3])
{
	float s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////