#pragma once

#include <cstdlib>

constexpr int B		= 256;
constexpr int BM	= 255;
constexpr int N		= 4096;

class OldPerlinNoise
{
public:
	static double noise1D(double arg);
	static double noise2D(float vec[2]);

private:
	///Initialisation///
	//Does the class need to be initialised
	static bool		start;

	//Set up the permuation and gradient tables
	static int	perm[B + B + 2];
	static float grad1D[B + B + 2];
	static float grad2D[B + B + 2][2];

	static void	init(void);
	static void	setup(float* vec, int i, int& b0, int& b1, float& r0, float& r1);

	///Helper math functions///
	static const void normalize2D(float v[2]);
	static const void normalize3D(float v[3]);

	//Easing interpolation
	static const inline float s_curve(float t)
	{
		// 3t^2 - 2t^3
		return (t * t * (3. - 2. * t));
	}

	//Linear interpolation
	static const inline float lerp(float t, float a, float b)
	{ 
		return (a + t * (b - a));
	}

	static const inline float dotProduct(float x1, float y1, float x2, float y2)
	{
		return  x1 * x2 + y1 * y2;
	}

	///Constructor/Destructor
	//Prevent instances being created
	OldPerlinNoise() {}
	~OldPerlinNoise() {}
};

