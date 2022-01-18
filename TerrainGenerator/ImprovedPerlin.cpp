/*
 * This is the Improved Prelin Noise class it handles:
 *		- Executing the Improved Perlin Noise algorithm
 *
 *
 * Original @author Abertay University.
 * Updated by @author D. Green.
 * 
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#include "ImprovedPerlin.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImprovedPerlin::start = true;
int ImprovedPerlin::p[512];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double ImprovedPerlin::noise(double x, double y, double z)
{
    if (start)
    {
        genPerm();
        start = false;
    }

    int X = (int)floor(x) & 255,                             // FIND UNIT CUBE THAT CONTAINS POINT.
        Y = (int)floor(y) & 255,
        Z = (int)floor(z) & 255;

    x -= floor(x);                                          // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
    y -= floor(y);
    z -= floor(z);

    double  u = fade(x),                                    // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.
        v = fade(y),
        w = fade(z);

    int A = p[X] + Y,
        AA = p[A] + Z,
        AB = p[A + 1] + Z,                                 // HASH COORDINATES OF THE 8 CUBE CORNERS,
        B = p[X + 1] + Y,
        BA = p[B] + Z,
        BB = p[B + 1] + Z;

    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),                // AND ADD
        grad(p[BA], x - 1, y, z)),           // BLENDED
        lerp(u, grad(p[AB], x, y - 1, z),            // RESULTS
            grad(p[BB], x - 1, y - 1, z))),      // FROM  8
        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),        // CORNERS
            grad(p[BA + 1], x - 1, y, z - 1)),   // OF CUBE
            lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const double ImprovedPerlin::fade(double t)
{
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6 - 15) + 10);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const double ImprovedPerlin::lerp(double t, double a, double b)
{
    return a + t * (b - a);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const double ImprovedPerlin::grad(int hash, double x, double y, double z)
{
    int h = hash & 15;                          // CONVERT LO 4 BITS OF HASH CODE
    double u = h < 8 ? x : y,                   // INTO 12 GRADIENT DIRECTIONS.
        v = h < 4 ? y : h == 12 || h == 14 ? x : z;

    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const void ImprovedPerlin::genPerm()
{
    for (int i = 0; i < 256; i++)
    {
        p[256 + i] = p[i] = permutation[i];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////