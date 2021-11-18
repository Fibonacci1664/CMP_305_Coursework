#include "App1.h"
#include <vector>

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
}

void App1::checkFaulting()
{
	if (loopFaulting)
	{
		terrainMesh->generateFault();
		terrainMesh->regenerateTerrain();
	}
	else if (runFaultingIterations && faultingIetrations > 0)
	{
		terrainMesh->generateFault();
		terrainMesh->regenerateTerrain();

		--faultingIetrations;

		if (faultingIetrations == 0)
		{
			runFaultingIterations = false;
		}
	}
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

	buildAllGuiOptions();

	// Get a brand new flat terrain
	if (ImGui::Button("Reset Terrain"))
	{
		terrainMesh->resetTerrain();
		terrainMesh->regenerateTerrain();
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::initGUIToggles()
{
	loopFaulting = false;
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

	// Ints
	faultingIetrations = 0;

	// Floats

}

void App1::buildAllGuiOptions()
{
	buildFaultingGui();
}

void App1::buildFaultingGui()
{
	if (ImGui::TreeNode("Faulting"))
	{
		if (ImGui::Button("Add Single Fault"))
		{
			terrainMesh->generateFault();
			terrainMesh->regenerateTerrain();
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
