#pragma once

class ParticleDeposition
{
public:
	ParticleDeposition(int& res, float* heightmp);
	~ParticleDeposition();
	void runParticleDepo();

private:
	void startParticleDepo();

	bool getNewStartPos;

	int& resolution;
	float* heightmap;
	int xPos;
	int zPos;
};

