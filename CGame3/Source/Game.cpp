

#include "Game.h"
#include "SimpleObj.h"

Game::Game() 
{
	OGLES.Init(); // make sure we initialise the OGL instance we called OGLES
}; 

Game::~Game() {}; // We should close down some things here but can do it later.

void Game::Update()
{
	
// start by creating some SimpleObj's and initialising some important variables/
	
	SimpleObj Bobby1; // create a Simple Object with the class name Bobby1, at 0.0f,0.0f and with speed 0.001f
	SimpleObj Bobby2;
//Give our Bobbies a name	
	Bobby1.m_MyName = "Bobby1"; // Give Bobby 1 his name
	Bobby2.m_MyName = "Bobby2"; // Give Bobby 2 his name
// they do a draw, so they need to know where the Program Object is which is in OGLES
	Bobby1.ProgramObject = OGLES.programObject;
	Bobby2.ProgramObject = OGLES.programObject; // set up the program object 
	
	Bobby1.Xpos = -0.75;
	Bobby2.Xpos =  0.76;	
	Bobby1.Ypos = Bobby2.Ypos = 0;

// now we will do a loop
	for (int i = 0; i < 1000; i++)
	{
		glClear(GL_COLOR_BUFFER_BIT); // clear the screen this time so we can see our black boxes
		Bobby1.Update(); // do Bobby1's update and draw
		Bobby2.Update(); // do Bobby2's update and draw
	
// the Bobbies have updated and drawn thier triangle at the right position, but they are currently
// all drawn and held in a surface buffer, so now lets tell EGL to swap that buffer to the display so we can see it.

		eglSwapBuffers(OGLES.state.display, OGLES.state.surface);

	} // this is the end of the for loop, the i will be incremented and tested to see if its still <100, if so, it repears the loop with i now +1
	return; 
};