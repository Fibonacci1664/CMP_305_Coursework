#pragma once

class ParticleDeposition
{
public:
	ParticleDeposition(int& res, float* heightmp);
	~ParticleDeposition();
	void runParticleDepo();
	void updateHeightMap(float* newHeightMap);

private:
	void startParticleDepo();

	bool getNewStartPos;

	int& resolution;
	float* heightmap;
	int xPos;
	int zPos;
};

