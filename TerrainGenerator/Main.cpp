/*
 * THIS WAS CREATED USING VISUAL STUDIO COMMUNITY EDITION - VERSION 16.11.6
 * AND DirectX 11
 */

 /*
  * This is the main point of entry for the program and handles
  *		- Setting the applications window size.
  *     - Creating and running the Application
  *
  * Original @author Abertay University.
  *	Updated by @author D. Green.
  */

 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 // INCLUDES
#include <cstdlib>
#include <ctime>
#include "System.h"
#include "App1.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	std::srand(std::time(nullptr));	// Seed the pseudo RNG

	App1* app = new App1();
	System* system;

	// Create the system object.
	system = new System(app, 1600, 900, true, false);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////