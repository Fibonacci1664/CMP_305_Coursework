/*
 * This is the main application file which constructs the entire scene, it handles:
 *		- Initialisation of all textures
 *		- Initialisation of all scene objects
 *		- Initialisation of all shaders
 *		- Initialisation of all lights
 * 
 *		- Processing GUI input to render various terrain features
 *
 *		- Rendering of the terrain, and L-System
 *		- Rendering and updating the GUI
 *
 * Original @author Abertay University.
 * Updated by @author D. Green.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES
#include <vector>
#include "App1.h"

// CONSTRUCTOR / DESTRUCTOR
App1::App1() : l_System("FA")
{
	noiseStyleValue = 0.0f;
	systems.emplace("3DCylTree", false);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	if (terrainMesh)
	{
		delete terrainMesh;
		terrainMesh = nullptr;
	}

	if (terrainShader)
	{
		delete terrainShader;
		terrainShader = nullptr;
	}

	if (dirLight)
	{
		delete dirLight;
		dirLight = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS
void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	loadTextures();

	// Create Mesh objects
	initSceneObjects();

	// Create shader objects
	initShaders(hwnd);

	// Initialise light
	initLights();

	// Initialise camera
	initCam();

	// Init all the variables used by the GUI
	initGUIVars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::loadTextures()
{
	textureMgr->loadTexture(L"brick", L"resources/textures/snow.png");
	textureMgr->loadTexture(L"grass", L"resources/textures/grass.png");
	textureMgr->loadTexture(L"sand", L"resources/textures/sand.png");
	textureMgr->loadTexture(L"water", L"resources/textures/water.png");
	textureMgr->loadTexture(L"bark", L"resources/textures/bark.png");
	textureMgr->loadTexture(L"leaf", L"resources/textures/leaf.png");
	/*textureMgr->loadTexture(L"goldLeaf", L"resources/textures/golden_leaf.png");
	textureMgr->loadTexture(L"goldBark", L"resources/textures/golden_bark.png");*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initSceneObjects()
{
	terrainMesh = new Terrain(renderer->getDevice(), renderer->getDeviceContext(), 512);		// Remember this is res NOT size, size is set in terrain.h
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initShaders(HWND& hwnd)
{
	terrainShader = new TerrainShader(renderer->getDevice(), hwnd);
	leafShader = new LeafShader(renderer->getDevice(), hwnd);
	lightShader = new LightShader(renderer->getDevice(), hwnd);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initLights()
{
	initDirLight();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initDirLight()
{
	dirLight = new Light();
	dirLight->setAmbientColour(0.2f, 0.2f, 0.2f, 1.0f);
	dirLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	dirLight->setDirection(0.7f, -0.7f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initCam()
{
	camera->setPosition(-259.91f, 167.86f, -237.27f);
	camera->setRotation(23.00f, 33.25f, 0.00f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initGUIVars()
{
	// BOOLS
	buildingFullTerrain = false;

	// For Smoothing
	loopSmoothing = false;
	runSmoothingIterations = false;
	smoothingValsSet = false;

	// For Faulting
	loopFaulting = false;
	runFaultingIterations = false;

	// For Particle Depo
	loopParticleDepo = false;
	runParticleDepoIterations = false;

	// For Perlin Noise and fBm
	newRandomNoise = false;
	addFixedNoise = false;
	ridgedPerlinToggle = false;
	terracedPerlinToggle = false;
	fBmToggle = false;
	fBmValsSet = false;
	runSingleOctave = false;
	runAllOctaves = false;

	// For Hydr Eros
	haveEroded = false;

	// INTS
	terrainResolution = 512;
	faultingIterations = 0;
	smoothingIterations = 0;
	particleDepoIterations = 0;
	fBmOctaves = 0;

	// FLOATS
	perlinFreq = 0.2f;
	perlinScale = 0.2f;
	amplitude = 5.0f;

	N_waterLowerBound = 0.0f;
	N_waterUpperbound = 3.0f;
	N_grassLowerBound = 3.0f;
	N_grassUpperBound = 7.0f;

	R_waterLowerBound = -15.0f;
	R_waterUpperbound = -5.0f;
	R_grassLowerBound = -5.0f;
	R_grassUpperBound = -0.5f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::updateTerrain()
{
	checkPerlinNoise();
	checkFaulting();
	checkSmoothing();
	checkParticleDepo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::checkFaulting()
{
	if (loopFaulting)
	{
		terrainMesh->generateFault();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
	}
	else if (runFaultingIterations && faultingIterations > 0)
	{
		terrainMesh->generateFault();

		--faultingIterations;

		if (faultingIterations == 0)
		{
			runFaultingIterations = false;
		}

		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::checkSmoothing()
{
	if (runSmoothingIterations && smoothingIterations > 0)
	{
		terrainMesh->smoothTerrain();

		--smoothingIterations;

		if (smoothingIterations == 0)
		{
			runSmoothingIterations = false;
		}

		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::checkParticleDepo()
{
	if (loopParticleDepo)
	{
		terrainMesh->startParticleDepo();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
	}
	else if (runParticleDepoIterations && particleDepoIterations > 0)
	{
		terrainMesh->startParticleDepo();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());

		--particleDepoIterations;

		if (particleDepoIterations == 0)
		{
			runParticleDepoIterations = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::checkPerlinNoise()
{
	if (addFixedNoise)
	{
		terrainMesh->setPNFreqScaleAmp(perlinFreq, perlinScale, amplitude);
		terrainMesh->genPerlinNoise();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
		addFixedNoise = false;
	}

	if (newRandomNoise)
	{
		// Get a blank terrain with 0 for height values
		terrainMesh->resetTerrain();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());

		// These max min rnage values must be the same as the max min GUI values
		const float MIN_RAND = 0.05f, MAX_RAND = 0.15f;
		const float range = MAX_RAND - MIN_RAND;
		float randomFreq = range * ((((double)rand()) / (double)RAND_MAX)) + MIN_RAND;
		float randomScale = range * ((((double)rand()) / (double)RAND_MAX)) + MIN_RAND;

		//// Rand num between 0 - 0.3
		//double newFreq = (((double)rand() / (double)RAND_MAX) / 100.0f) + 0.1f;
		//// Rand num btween 0 - 0.3
		//double newScale = (((double)rand() / (double)RAND_MAX) / 100.0f) + 0.1f;
		// Rand num between 5 - 10
		double randomAmp = rand() % 5 + 5;

		perlinFreq = randomFreq;
		perlinScale = randomScale;
		amplitude = randomAmp;

		terrainMesh->setPNFreqScaleAmp(perlinFreq, perlinScale, amplitude);
		terrainMesh->genPerlinNoise();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());

		newRandomNoise = false;
	}

	if (runSingleOctave)
	{
		terrainMesh->generatefBm();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
		// Control entry into this statement unless the button is clicked again
		runSingleOctave = false;
	}

	if (runAllOctaves && fBmOctaves > 0)
	{
		terrainMesh->generatefBm();
		// Update the slider values with the PN class values that have been getting halved and doubled
		perlinFreq = terrainMesh->getPerlinFreq();
		amplitude = terrainMesh->getPerlinAmplitude();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());

		--fBmOctaves;

		if (fBmOctaves <= 0)
		{
			// Reset the values to the defaults
			perlinFreq = 0.2f;
			perlinScale = 0.2f;
			amplitude = 5.0f;
			terrainMesh->setPNFreqScaleAmp(perlinFreq, perlinScale, amplitude);
			fBmOctaves = 0;
			runAllOctaves = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildLSystem()
{
	//Get the current L-System string, right now we have a place holder
	std::string systemString = l_System.GetCurrentSystem();

	//Initialise some variables
	XMVECTOR fwd = XMVectorSet(0, 0, 1, 0);		//Rotation axis. Our rotations happen around the "forward" vector
	XMVECTOR left = XMVectorSet(-1, 0, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);		//Current direction is "Up" Having 0 as the 'w' val prevents the coords of this 'vector' being modified by translations
	XMVECTOR pos = XMVectorSet(0, 0, 0, 1);		//Current position (0,0,0) Having 1 as the 'w' val allows the coords of this 'point' to be modified by translations
	XMVECTOR dir = XMVectorSet(0, 1, 0, 0);		//Current position (0,0,0) Having 1 as the 'w' val allows the coords of this 'point' to be modified by translations

	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(0, 0, 0);

	// Go through the L-System string
	for (int i = 0; i < systemString.length(); ++i)
	{
		if (build3DCylTreeToggle)
		{
			buildCyl3DTree(systemString[i], pos, dir, up, fwd, left, currentRotation);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildCyl3DTree(char letter, XMVECTOR& pos, XMVECTOR& dir, XMVECTOR& up, XMVECTOR& fwd, XMVECTOR& left, XMMATRIX& currRot)
{
	float pitch = 25.0f;
	float theta = 120.0f;
	int randTheta = rand() % 40 + 80;		// Random num between 60 - 120
	int randPitch = rand() % 10 + 25;		// Random num between 5 - 25

	float randomMultiplier = ((float)rand()) / (float)RAND_MAX;

	if (randomMultiplier < 0.5f)
	{
		randomMultiplier += 0.5f;
	}

	float randFloatTheta = (float)randTheta * randomMultiplier;
	float randFloatPitch = (float)randPitch * randomMultiplier;

	switch (letter)
	{
	case 'A':
		// Add a leaf, a textured quad with transparent leaf tex, clip the transparent pixels in the PS
		// This ensures we're not adding leaves to low down the trunk
		if (iterations > 3)
		{
			// 50/50 whether or not to add a leaf
			if (rand() % 2)
			{
				addLeaf(pos, currRot);
			}
		}

		break;
	case 'F':
		XMVECTOR newBranchLength = XMVectorScale(dir, branchLengthMult);	// Get the new branch length based off of the multiplier reduction and using the dir vector
		addCylinder(pos, currRot, newBranchLength, btmRad, topRad);			// Add a branch at position, with current rotation, with the new length, with the top and btm radius specified
		pos += XMVector3TransformNormal(newBranchLength, currRot);			// Move to the end of the branch
		break;
	case '[':				// Save
		position.push(pos);						// Save the current position
		rotation.push(currRot);					// Save the current rotation
		branchLength.push(branchLengthMult);	// Save current branch length
		topRadStk.push(topRad);					// Save the current top rad
		btmRadStk.push(btmRad);					// Save the current btm rad
		branchLengthMult *= 0.68f;				// Reduce the branch length
		btmRad = topRad;						// Set the btm radius to be the current top rad, this is so the next branch joins perfectly with the correct radius
		topRad *= 0.60;							// Reduce the top radius of the branches so they get ever thinner
		break;
	case ']':				// Restore
		pos = position.top();					// Reset the position to that which was saved
		position.pop();							// Pop the top of the position stack
		currRot = rotation.top();				// Reset the rotation to that which was saved
		rotation.pop();							// Pop the top of the rotation stack
		branchLengthMult = branchLength.top();	// Same same for all the rest
		branchLength.pop();
		topRad = topRadStk.top();
		topRadStk.pop();
		btmRad = btmRadStk.top();
		btmRadStk.pop();
		break;
	case '&':				// Pitch
		currRot *= XMMatrixRotationAxis(XMVector3TransformNormal(left, currRot), AI_DEG_TO_RAD(randFloatPitch));	// Rotate around left vector (x axis)
		break;
	case '>':				// Rotate right
		currRot *= XMMatrixRotationAxis(XMVector3Transform(dir, currRot), AI_DEG_TO_RAD(-randFloatTheta));			// Rotate around up vector (y axis)
		break;
	case '<':				// Rotate left
		currRot *= XMMatrixRotationAxis(XMVector3Transform(dir, currRot), AI_DEG_TO_RAD(randFloatTheta));			// Rotate around up vector (y axis)
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::addCylinder(XMVECTOR& pos, XMMATRIX& currRot, XMVECTOR branchLen, float btmRadius, float topRadius)
{
	float len = XMVectorGetX(XMVector3Length(branchLen));

	m_Cylinder = new CylinderMesh(renderer->getDevice(), renderer->getDeviceContext(), 1.0f, 6.0f, len, btmRadius, topRadius);
	m_Cylinder->m_Transform = currRot * XMMatrixTranslationFromVector(pos);
	m_CylinderList.push_back(m_Cylinder);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::addLeaf(XMVECTOR& pos, XMMATRIX& currRot)
{
	// FOR CUSTOM LEAF
	float leafScale = 0.02f;

	quadLeaf = new Leaf(renderer->getDevice(), renderer->getDeviceContext(), pos, leafScale);
	quadLeaf->m_transform = currRot;
	leafList.push_back(quadLeaf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::resetLSystem()
{
	iterations = 0;
	branchLengthMult = 1.0f;
	l_System.Reset();

	for (int i = 0; i < m_CylinderList.size(); ++i)
	{
		delete m_CylinderList[i];
		m_CylinderList[i] = nullptr;
	}

	for (int i = 0; i < leafList.size(); ++i)
	{
		delete leafList[i];
		leafList[i] = nullptr;
	}

	m_CylinderList.clear();
	leafList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();

	if (!result)
	{
		return false;
	}

	updateTerrain();
	
	// Render the graphics.
	result = render();

	if (!result)
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool App1::render()
{
	// Clear the scene. (default blue colour)
	//renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	renderTerrain();
	renderLSystem();

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::renderTerrain()
{
	XMFLOAT4 normalTextBoundValues;
	normalTextBoundValues.x = N_waterLowerBound;
	normalTextBoundValues.y = N_waterUpperbound;
	normalTextBoundValues.z = N_grassLowerBound;
	normalTextBoundValues.w = N_grassUpperBound;

	XMFLOAT4 ridgedTextBoundValues;
	ridgedTextBoundValues.x = R_waterLowerBound;
	ridgedTextBoundValues.y = R_waterUpperbound;
	ridgedTextBoundValues.z = R_grassLowerBound;
	ridgedTextBoundValues.w = R_grassUpperBound;

	worldMatrix *= XMMatrixTranslation(-125.0f, 2.0f, -125.0f);

	terrainMesh->sendData(renderer->getDeviceContext());

	terrainShader->setShaderParameters(renderer->getDeviceContext(),
		worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"snow"), textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"water"),
		dirLight, noiseStyleValue, normalTextBoundValues, ridgedTextBoundValues);

	terrainShader->render(renderer->getDeviceContext(), terrainMesh->getIndexCount());

	worldMatrix = XMMatrixIdentity();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::renderLSystem()
{
	// FOR CYL TREE
	for (int i = 0; i < m_CylinderList.size(); ++i)
	{
		if (m_CylinderList[i]->getIndexCount() > 0)
		{
			worldMatrix = XMMatrixMultiply(m_CylinderList[i]->m_Transform, worldMatrix);	
			worldMatrix *= XMMatrixScaling(20.0f, 20.0f, 20.0f);

			m_CylinderList[i]->sendData(renderer->getDeviceContext());
			lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"bark"), dirLight);
			//lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"goldBark"), dirLight);
			lightShader->render(renderer->getDeviceContext(), m_CylinderList[i]->getIndexCount());

			worldMatrix = XMMatrixIdentity();
		}
	}
			
	// FOR CUSTOM LEAF
	for (int i = 0; i < leafList.size(); ++i)
	{
		if (leafList.size() > 0)
		{
			if (leafList[i]->getIndexCount() > 0)
			{
				worldMatrix = XMMatrixRotationAxis(leafList[i]->m_position, -5.0f);				
				worldMatrix *= XMMatrixScaling(20.0f, 20.0f, 20.0f);

				leafList[i]->sendData(renderer->getDeviceContext());
				leafShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"leaf"), dirLight);				
				//leafShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"goldLeaf"), dirLight);				
				leafShader->render(renderer->getDeviceContext(), leafList[i]->getIndexCount());

				worldMatrix = XMMatrixIdentity();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	//ImGui::ShowDemoWindow();

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Terrain Resolution: %d", terrainMesh->getTerrainRes());
	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Text("Camera Rot: (%.2f, %.2f, %.2f)", camera->getRotation().x, camera->getRotation().y, camera->getRotation().z);
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	//ImGui::SliderInt("Terrain Resolution", &terrainResolution, 512, 1024);

	// Resize the terrain to a new resolution
	/*if (ImGui::Button("Resize Terrain"))
	{
		if (terrainResolution != terrainMesh->getTerrainRes())
		{
			terrainMesh->resize(terrainResolution);
			terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
			terrainMesh->updateHeightMap();
		}
	}*/

	// Get a brand new flat terrain
	if (ImGui::Button("Reset Terrain"))
	{
		terrainMesh->resetTerrain();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
	}

	buildSmoothingGui();
	buildAllGuiOptions();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildAllGuiOptions()
{
	if (ImGui::TreeNode("Hydraulic Erosion"))
	{
		buildHydErosionGui();

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Build Complete Terrain"))
	{
		buildCompleteTerrainGui();

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Build L-System Tree"))
	{
		buildLSystemGUI();

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Play with Terrain Features"))
	{
		buildFaultingGui();
		buildParticleDepoGui();
		buildPerlinNoiseGui();

		ImGui::TreePop();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildHydErosionGui()
{
	ImGui::Text("* NOTE *	YOU MUST BUILD A TERRAIN FIRST!");
	ImGui::Text("* NOTE *	HYDRAULIC EROSION WORKS WITH:");
	ImGui::Text("			- NORMAL NOISE TERRAIN");
	ImGui::Text("			- TERRACED NOISE TERRAIN");
	ImGui::Text(" ");
	ImGui::Text("Change these values to change the look of the erosion\n");

	ImGui::SliderInt("Cycles", &erosionIterations, 250000, 500000);
	ImGui::SliderInt("Erosion Radius", &erosionRadius, 2.0, 10.0);
	ImGui::SliderFloat("Inertia", &inertia, 0.001, 0.999);
	ImGui::SliderFloat("Sediment Capacity", &sedimentCapacity, 1.0, 5.0);
	ImGui::SliderFloat("Erosion Speed", &erodeSpeed, 0.001, 0.999);
	ImGui::SliderFloat("Deposition Speed", &depositSpeed, 0.001, 0.999);
	ImGui::SliderFloat("Water Evaporation Speed", &evaporateSpeed, 0.001, 0.999);
	//ImGui::SliderFloat("Gravity", &gravity, 4.0, 10.0);
	ImGui::SliderInt("Max Particle Lifetime", &maxDropletLifetime, 5, 50);

	terrainMesh->setErosionRad(erosionRadius);
	terrainMesh->setInertia(inertia);
	terrainMesh->setSedimentCap(sedimentCapacity);
	terrainMesh->setErosionSpeed(erodeSpeed);
	terrainMesh->setDepositSpeed(depositSpeed);
	terrainMesh->setEvapSpeed(evaporateSpeed);
	//terrainMesh->setGravity(gravity);
	terrainMesh->setMaxParticleLifetime(maxDropletLifetime);

	if (ImGui::Button("Erode Terrain"))
	{
		ImGui::Text("Please Wait...");
		terrainMesh->erodeTerrain(erosionIterations);
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());

		// Hard set the texture bounds for a textured terrain with white shorelines to imitate sea foam
		adjustedTextureBounds();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildCompleteTerrainGui()
{
	ImGui::Text("* NOTE *	ENSURE PERLIN NOISE TREE IS OPEN IN TERRAIN FEATURES BELOW");
	ImGui::Text(" ");

	if (ImGui::Button("Build Complete Terrain"))
	{
		buildingFullTerrain = true;
		smoothingValsSet = false;
		fBmValsSet = false;
		haveEroded = false;

		// Reset the texture bounds to defaults when generating a new terrian
		initialTextureBounds();

		// ########################### ADD NOISE ###########################
		newRandomNoise = true;
		// ########################### ADD FAULTING ###########################
		// Random value
		faultingIterations = rand() % 150 + 50;
		// Set Value
		//faultingIterations = 200;
		runFaultingIterations = true;
	}

	if (buildingFullTerrain)
	{
		// If we're finished faulting, AND we have not yet set smoothing values, then do so
		if (!runFaultingIterations && !smoothingValsSet)
		{
			smoothingValsSet = true;
			// ########################### ADD SMOOTHING ###########################
			// Random value
			//smoothingIterations = rand() % 75;
			// Set Value
			smoothingIterations = 75;
			runSmoothingIterations = true;
		}

		// If we're finished faulting AND smoothing, AND we have not yet set fBm values, then do so
		if (!runFaultingIterations && !runSmoothingIterations && !fBmValsSet)
		{
			fBmValsSet = true;

			// ########################### ADD FBM ###########################
			// Random value
			//fBmOctaves = rand() % 10 + 1;
			// Set Value
			fBmOctaves = 8;
			runAllOctaves = true;
		}

		// If we're finished faulting AND smoothing AND fBm, AND we have not carried out erosion yet, then do so
		if (!runFaultingIterations && !runSmoothingIterations && !runAllOctaves && !haveEroded)
		{
			haveEroded = true;

			// ########################### ADD HYDRAULIC EROSION ###########################
			terrainMesh->erodeTerrain(erosionIterations);
			terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());

			// Hard set the texture bounds for a textured terrain with white shorelines to imitate sea foam
			adjustedTextureBounds();

			buildingFullTerrain = false;
		}
	}

	ImGui::Text("Use with Normal or Terraced Noise Height Map");
	ImGui::SliderFloat("Water Lower Bound", &N_waterLowerBound, -20.0, 20.0);
	ImGui::SliderFloat("Water Upper Bound", &N_waterUpperbound, -20.0, 20.0);
	ImGui::SliderFloat("Grass Lower Bound", &N_grassLowerBound, -20.0, 20.0);
	ImGui::SliderFloat("Grass Upper Bound", &N_grassUpperBound, -20.0, 100.0);

	ImGui::Text("Use with Ridged Noise Height Map");
	ImGui::SliderFloat("Ridged Water Lower Bound", &R_waterLowerBound, -20.0, 20.0);
	ImGui::SliderFloat("Ridged Water Upper Bound", &R_waterUpperbound, -25.0, 20.0);
	ImGui::SliderFloat("Ridged Grass Lower Bound", &R_grassLowerBound, -20.0, 20.0);
	ImGui::SliderFloat("Ridged Grass Upper Bound", &R_grassUpperBound, -20.0, 100.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildSmoothingGui()
{
	if (ImGui::Button("Smooth Terrain"))
	{
		terrainMesh->smoothTerrain();
		terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
	}

	// This slider lets the user control how many iterations of the faulting algorithm they wish to run
	ImGui::SliderInt("Smoothing Iterations", &smoothingIterations, 2, 1000);

	if (ImGui::Button("Run All Smoothing Iterations"))
	{
		// Reset the texture bounds to defaults when generating a new terrian
		initialTextureBounds();

		if (!loopSmoothing)
		{
			runSmoothingIterations = true;
		}
		else
		{
			runSmoothingIterations = false;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildLSystemGUI()
{

	ImGui::Checkbox("Build 3D Cylinder Tree", &build3DCylTreeToggle);
	systems["3DCylTree"] = build3DCylTreeToggle;

	if (ImGui::Button("Reset L-System"))
	{
		resetLSystem();
	}

	if (ImGui::Button("Build Entire Tree"))
	{
		resetLSystem();

		while (iterations < 8)
		{
			buildLSystem();
			l_System.Iterate(systems);
			++iterations;
		}
	}

	if (ImGui::Button("Iterate Over Tree"))
	{
		buildLSystem();
		l_System.Iterate(systems);
		++iterations;
	}

	ImGui::Text("Iterations: %d", iterations);

	ImGui::LabelText(l_System.GetAxiom().c_str(), "Axiom:");

	// FIX THIS IT IS NOT SHOWING THE CORRECT SYSTEM FOR THE CURRENT ITERATION! IT IS SHOWING THE NEXT SYSTEM!
	ImGui::Text("System:");
	ImGui::TextWrapped(l_System.GetCurrentSystem().c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildFaultingGui()
{
	if (ImGui::TreeNode("Faulting"))
	{
		if (ImGui::Button("Add Single Fault"))
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			terrainMesh->generateFault();
			terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
		}

		ImGui::Text("Loop Faulting");

		// These radio buttons will loop the faulting algorithm indefinately until switched OFF again
		// If switched to the ON state while running an amount of set iterations
		// the iterations slider will move into a pause state
		static int state = 0;

		ImGui::RadioButton("Off", &state, 0); ImGui::SameLine(); ImGui::RadioButton("On", &state, 1);
		
		if (state)
		{
			loopFaulting = true;

			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();
		}
		else
		{
			loopFaulting = false;
		}

		// This slider lets the user control how many iterations of the faulting algorithm they wish to run
		ImGui::SliderInt("Faulting Iterations", &faultingIterations, 2, 1000);
		
		if (ImGui::Button("Run All Iterations"))
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			if (!loopFaulting)					
			{
				runFaultingIterations = true;
			}
			else
			{
				runFaultingIterations = false;
			}
		}

		// Manually pause the set iterations
		if (ImGui::Button("Pause"))
		{
			runFaultingIterations = false;
		}

		ImGui::TreePop();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildParticleDepoGui()
{
	if (ImGui::TreeNode("Particle Deposition"))
	{
		if (ImGui::Button("Add Single Particle"))
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			terrainMesh->startParticleDepo();
			terrainMesh->generateTerrain(renderer->getDevice(), renderer->getDeviceContext());
		}

		ImGui::Text("Loop Particle Deposition");

		// These radio buttons will loop the particle depo algorithm indefinately until switched OFF again
		// If switched to the ON state while running an amount of set iterations
		// the iterations slider will move into a pause state
		static int state = 0;

		ImGui::RadioButton("Off", &state, 0); ImGui::SameLine(); ImGui::RadioButton("On", &state, 1);

		if (state)
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			loopParticleDepo = true;
		}
		else
		{
			loopParticleDepo = false;
		}

		// This slider lets the user control how many iterations of the faulting algorithm they wish to run
		ImGui::SliderInt("Particle Deposition Iterations", &particleDepoIterations, 2, 1000);

		if (ImGui::Button("Run All Iterations"))
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			if (!loopParticleDepo)
			{
				runParticleDepoIterations = true;
			}
			else
			{
				runParticleDepoIterations = false;
			}
		}

		// Manually pause the set iterations
		if (ImGui::Button("Pause"))
		{
			runParticleDepoIterations = false;
		}

		ImGui::TreePop();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::buildPerlinNoiseGui()
{
	if (ImGui::TreeNode("Perlin Noise"))
	{
		ImGui::SliderFloat("Amplitude", &amplitude, 5.0f, 15.0f);
		ImGui::SliderFloat("Frequency", &perlinFreq, 0.05f, 0.15f);
		ImGui::SliderFloat("Scale", &perlinScale, 0.05f, 0.15f);

		static int algoState = 0;

		ImGui::RadioButton("Old PN Algo", &algoState, 0); ImGui::SameLine(); ImGui::RadioButton("Improved PN Algo", &algoState, 1);

		if (algoState)
		{
			// 'I' == Improved algorithm
			terrainMesh->setPerlinAlgoType('I');
		}
		else
		{
			// 'O' == Old algorithm
			terrainMesh->setPerlinAlgoType('O');
		}

		static int noiseStyle = 0;

		if (ImGui::RadioButton("Normal Noise", &noiseStyle, 0))
		{
			ridgedPerlinToggle = false;
			terracedPerlinToggle = false;
			noiseStyleValue = 0;
		}

		if (ImGui::RadioButton("Ridged Noise", &noiseStyle, 1))
		{
			ridgedPerlinToggle = true;
			terracedPerlinToggle = false;
			noiseStyleValue = 1;
		}

		if(ImGui::RadioButton("Terraced Noise", &noiseStyle, 2))
		{
			ridgedPerlinToggle = false;
			terracedPerlinToggle = true;
			noiseStyleValue = 2;
		}

		terrainMesh->setPerlinRidged(ridgedPerlinToggle);
		terrainMesh->setPerlinTerraced(terracedPerlinToggle);

		// Cumulatively add noise to the existing noise map with the current freq and scale vals
		ImGui::Text("This Cumulatively Adds To The Existing");
		ImGui::Text("Map Using The Noise Type Selected");
		if (ImGui::Button("Add Fixed Noise Using Current Freq/Scale Values"))
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			addFixedNoise = true;
		}

		//ImGui::SameLine();

		// Generate a new random map
		ImGui::Text("This Generates An Entirely New Map With Random");
		ImGui::Text("Freq/Scale Using The Noise Type Selected");
		if (ImGui::Button("Generate New Random Noise"))
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			newRandomNoise = true;
		}

		//ImGui::SameLine();

		// ############################### FRACTIONAL BROWNIAN MOTION STUFF ###############################

		ImGui::Text("Uncheck This Box to Alter Terrain Texture Heights");
		ImGui::Checkbox("Use fBm", &fBmToggle);
		
		if (fBmToggle)
		{
			// Reset the texture bounds to defaults when generating a new terrian
			initialTextureBounds();

			if (ImGui::Button("Run Single fBm Octave"))
			{
				runSingleOctave = true;
			}

			ImGui::SliderInt("Octaves", &fBmOctaves, 0, 10);

			if (ImGui::Button("Run All fBm Octaves"))
			{
				runAllOctaves = true;
			}
		}

		ImGui::TreePop();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::initialTextureBounds()
{
	N_waterLowerBound = 0.0f;
	N_waterUpperbound = 3.0f;
	N_grassLowerBound = 3.0f;
	N_grassUpperBound = 7.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App1::adjustedTextureBounds()
{
	N_waterLowerBound = 1.921f;
	N_waterUpperbound = 3.0f;
	N_grassLowerBound = 3.0f;
	N_grassUpperBound = 15.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////