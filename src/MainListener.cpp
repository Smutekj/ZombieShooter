// #include "MainListener.h"
// #include "InputHandler.h"
// #include "World.h"
// #include <ois.h>


// MainListener::MainListener(Ogre::RenderWindow *mainWindow, InputHandler *inputManager, World *world) :
// mRenderWindow(mainWindow), mInputHandler(inputManager), mWorld(world)
// {
// }


// // On every frame, call the appropriate managers
// bool 
// 	MainListener::frameStarted(const Ogre::FrameEvent &evt)
// {
// 	float time = evt.timeSinceLastFrame;
// 	if (time > 0.5)
// 	{
// 		time = 0.5;
// 	}
// 	mInputHandler->Think(time);
// 	mWorld->Think(time);

// 	// Call think methods on any other managers / etc you want to add

// 	bool keepGoing = true;

// 	// Ogre will shut down if a listener returns false.  We will shut everything down if
// 	// either the user presses escape or the main render window is closed.  Feel free to 
// 	// modify this as you like.
// 	if (mInputHandler->isKeyDown(OIS::KC_ESCAPE) || mRenderWindow->isClosed())
// 	{
// 		keepGoing = false;
// 	}

// 	return keepGoing;
// }