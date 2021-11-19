#include <vector>

#include "App1.h"

App1::App1()
{
	
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	loadTextures();
	
	// Create Mesh objects
	initTerrain();

	// Create shader objects
	initLightShader(hwnd);

	// Initialise light
	initLights();

	// Initialise camera
	initCam();

	// Init all the variables used by the GUI
	initGUIVars();
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

	if (lightShader)
	{
		delete lightShader;
		lightShader = nullptr;
	}

	if (dirLight)
	{
		delete dirLight;
		dirLight = nullptr;
	}
}

void App1::updateTerrain()
{
	checkFaulting();
	checkParticleDepo();
	checkPerlinNoise();
}

void App1::checkFaulting()
{
	if (loopFaulting)
	{
		terrainMesh->generateFault();
	}
	else if (runFaultingIterations && faultingIetrations > 0)
	{
		terrainMesh->generateFault();

		--faultingIetrations;

		if (faultingIetrations == 0)
		{
			runFaultingIterations = false;
		}
	}

	terrainMesh->regenerateTerrain();
}

void App1::checkParticleDepo()
{
	if (loopParticleDepo)
	{
		terrainMesh->startParticleDepo();
	}
	else if (runParticleDepoIterations && particleDepoIterations > 0)
	{
		terrainMesh->startParticleDepo();

		--particleDepoIterations;

		if (particleDepoIterations == 0)
		{
			runParticleDepoIterations = false;
		}
	}

	terrainMesh->regenerateTerrain();
}

void App1::checkPerlinNoise()
{
	/*perlinFreq = terrainMesh->getPerlinFreq();
	amplitude = terrainMesh->getPerlinAmplitude();*/
}

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

bool App1::render()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	renderTerrain();

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	ImGui::ShowDemoWindow();

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::SliderInt("Terrain Resolution", &terrainResolution, 2, 1024);

	// Resize the terrain to a new resolution
	if (ImGui::Button("Resize Terrain"))
	{
		if (terrainResolution != terrainMesh->getTerrainRes())
		{
			terrainMesh->resize(terrainResolution);
			terrainMesh->regenerateTerrain();
			terrainMesh->updateHeightMap();
		}
	}

	// Get a brand new flat terrain
	if (ImGui::Button("Reset Terrain"))
	{
		terrainMesh->resetTerrain();
		terrainMesh->regenerateTerrain();
	}

	buildAllGuiOptions();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::loadTextures()
{
	textureMgr->loadTexture(L"brick", L"resources/textures/snow.png");
	textureMgr->loadTexture(L"grass", L"resources/textures/grass.png");
}

void App1::initTerrain()
{
	terrainMesh = new Terrain(renderer->getDevice(), renderer->getDeviceContext());
}

void App1::initLightShader(HWND& hwnd)
{
	lightShader = new LightShader(renderer->getDevice(), hwnd);
}

void App1::initLights()
{
	initDirLight();
}

void App1::initDirLight()
{
	dirLight = new Light();
	dirLight->setAmbientColour(0.2f, 0.2f, 0.2f, 1.0f);
	dirLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	dirLight->setDirection(0.7f, -0.7f, 0.0f);
}

void App1::initCam()
{
	camera->setPosition(-16, 60, -15);
	camera->setRotation(25, 45, 0);
}

void App1::initGUIVars()
{
	// Bools
	loopFaulting = false;
	runFaultingIterations = false;
	loopParticleDepo = false;
	runParticleDepoIterations = false;
	ridgedPerlinToggle = false;
	fBmToggle = false;

	// Ints
	terrainResolution = 128;
	faultingIetrations = 0;
	particleDepoIterations = 0;
	fBmOctaves = 0;

	// Floats
	perlinFreq = 0.25f;
	perlinScale = 0.05f;
	amplitude = 2.0f;
}

void App1::buildAllGuiOptions()
{
	buildFaultingGui();
	buildParticleDepoGui();
	buildPerlinNoiseGui();
}

void App1::buildFaultingGui()
{
	if (ImGui::TreeNode("Faulting"))
	{
		if (ImGui::Button("Add Single Fault"))
		{
			terrainMesh->generateFault();
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
		}
		else
		{
			loopFaulting = false;
		}

		// This slider lets the user control how many iterations of the faulting algorithm they wish to run
		ImGui::SliderInt("Faulting Iterations", &faultingIetrations, 2, 1000);
		
		if (ImGui::Button("Run All Iterations"))
		{										
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

void App1::buildParticleDepoGui()
{
	if (ImGui::TreeNode("Particle Deposition"))
	{
		if (ImGui::Button("Add Single Particle"))
		{
			terrainMesh->startParticleDepo();
		}

		ImGui::Text("Loop Particle Deposition");

		// These radio buttons will loop the particle depo algorithm indefinately until switched OFF again
		// If switched to the ON state while running an amount of set iterations
		// the iterations slider will move into a pause state
		static int state = 0;

		ImGui::RadioButton("Off", &state, 0); ImGui::SameLine(); ImGui::RadioButton("On", &state, 1);

		if (state)
		{
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

void App1::buildPerlinNoiseGui()
{
	if (ImGui::TreeNode("Perlin Noise"))
	{
		ImGui::SliderFloat("Amplitude", &amplitude, 2.0f, 50.0f);
		ImGui::SliderFloat("Frequency", &perlinFreq, 0.25f, 1.0f);
		ImGui::SliderFloat("Scale", &perlinScale, 0.05f, 0.1f);

		// Set PN class values according to the slider values
		terrainMesh->setPNFreqScaleAmp(perlinFreq, perlinScale, amplitude);

		static int state = 0;

		ImGui::RadioButton("Old PN Algo", &state, 0); ImGui::SameLine(); ImGui::RadioButton("Improved PN Algo", &state, 1);

		if (state)
		{
			// 'I' == Improved algorithm
			terrainMesh->setPerlinAlgoType('I');
		}
		else
		{
			// 'O' == Old algorithm
			terrainMesh->setPerlinAlgoType('O');
		}

		ImGui::Checkbox("Make Ridged Noise", &ridgedPerlinToggle);
		terrainMesh->setPerlinRidged(ridgedPerlinToggle);

		// Cumulatively add noise to the existing noise map with the current freq and scale vals
		if (ImGui::Button("Generate/Add Fixed Noise"))
		{
			terrainMesh->genPerlinNoise();
			terrainMesh->regenerateTerrain();
		}

		ImGui::SameLine();		ImGui::Text("This Cumulatively Adds To The Existing Map");

		// Generate a new random map
		if (ImGui::Button("Generate New Random Noise"))
		{
			terrainMesh->resetTerrain();
			terrainMesh->regenerateTerrain();

			// Rand num between 0 - 1
			double newFreq = (double)rand() / (double)RAND_MAX;
			// Rand num btween 0 - 0.1
			double newScale = ((double)rand() / (double)RAND_MAX) / 10.0f;

			perlinFreq = newFreq;
			perlinScale = newScale;
	
			terrainMesh->genPerlinNoise();
			terrainMesh->regenerateTerrain();
		}

		ImGui::SameLine();		ImGui::Text("This Generates An Entirely New Map With Random Freq/Scale");

		// ############################### FRACTIONAL BROWNIAN MOTION STUFF ###############################

		ImGui::Checkbox("Use fBm", &fBmToggle);
		
		if (fBmToggle)
		{
			ImGui::SliderInt("Octaves", &fBmOctaves, 0, 10);

			terrainMesh->setfBmOctaves(fBmOctaves);

			if (ImGui::Button("Run Single fBm"))
			{
				terrainMesh->setStepfBm(true);
				terrainMesh->generatefBm();
				terrainMesh->regenerateTerrain();
			}

			if (ImGui::Button("Run All fBm"))
			{
				terrainMesh->setStepfBm(false);
				terrainMesh->generatefBm();
				terrainMesh->regenerateTerrain();
			}
		}

		ImGui::TreePop();
	}
}

void App1::renderTerrain()
{
	// Add these later
	//textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"snow"),

	terrainMesh->sendData(renderer->getDeviceContext());

	lightShader->setShaderParameters(renderer->getDeviceContext(),
		worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"grass"),
		dirLight);

	lightShader->render(renderer->getDeviceContext(), terrainMesh->getIndexCount());
}
