#include "MixnMapApp.h"

void MixnMapApp::prepareSettings(Settings *settings)
{
	// instanciate the logger class
	log = Logger::create("MixnMaplog.txt");
	log->logTimedString("start");
	mLogMsg = "";
	// parameters
	mParameterBag = ParameterBag::create();
	getWindowsResolution();
#ifdef _DEBUG
	// debug mode
	settings->setWindowSize(mParameterBag->mRenderWidth / 2, mParameterBag->mRenderHeight / 2);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX -mParameterBag->mRenderWidth / 2, mParameterBag->mRenderY + 50));
#else
	settings->setWindowSize(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX - mParameterBag->mRenderWidth, mParameterBag->mRenderY));
#endif  // _DEBUG
	settings->setResizable(true); // allowed for a receiver
	settings->setFrameRate(1000.0f);
	if (mParameterBag->mShowConsole) settings->enableConsoleWindow();

}
void MixnMapApp::getWindowsResolution()
{
	newLogMsg = false;
	log->logTimedString("getWindowsResolution");
	mParameterBag->mDisplayCount = 0;
	// Display sizes
	mParameterBag->mMainDisplayWidth = Display::getMainDisplay()->getWidth();
	mParameterBag->mRenderX = mParameterBag->mMainDisplayWidth;
	mParameterBag->mRenderY = 0;
	for (auto display : Display::getDisplays())
	{
		mParameterBag->mDisplayCount++;
		mParameterBag->mRenderWidth = display->getWidth();
		mParameterBag->mRenderHeight = display->getHeight();
		log->logTimedString("Window " + toString(mParameterBag->mDisplayCount) + ": " + toString(mParameterBag->mRenderWidth) + "x" + toString(mParameterBag->mRenderHeight));
	}
	mLogMsg = " mRenderWidth" + toString(mParameterBag->mRenderWidth) + " mRenderHeight" + toString(mParameterBag->mRenderHeight + "\n");
	newLogMsg = true;
	log->logTimedString(" mRenderWidth" + toString(mParameterBag->mRenderWidth) + "mRenderHeight" + toString(mParameterBag->mRenderHeight));
	mParameterBag->mRenderResoXY = vec2(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);

	// in case only one screen , render from x = 0
	if (mParameterBag->mDisplayCount == 1) mParameterBag->mRenderX = 0;
}

void MixnMapApp::setup()
{
	log->logTimedString("setup");
	// in multi-window mode prevent shutdown to be executed twice
	mIsShutDown = false;
	// instanciate the Shaders class, must not be in prepareSettings
	mShaders = Shaders::create(mParameterBag);
	// instanciate the textures class
	mTextures = Textures::create(mParameterBag, mShaders);
	// instanciate the spout class
	mSpout = SpoutWrapper::create(mParameterBag, mTextures);
	// instanciate the OSC class
	mOSC = OSC::create(mParameterBag);
	windowManagement();
	// Setup the MinimalUI user interface
	mUI = UI::create(mParameterBag, mShaders, mTextures, mMainWindow);
	mUI->setup();
	// instanciate the warp wrapper class
	mWarpings = WarpWrapper::create(mParameterBag, mTextures, mShaders);


#ifdef _DEBUG
	// debug mode
#else
	hideCursor();
#endif  // _DEBUG	
	log->logTimedString("setup done");
}
void MixnMapApp::windowManagement()
{
	log->logTimedString("windowManagement");
	getWindowsResolution();
	// setup the main window and associated draw function
	mMainWindow = getWindow();
	mMainWindow->setTitle("Reymenta mix-n-map");
	mMainWindow->connectDraw(&MixnMapApp::drawMain, this);
	mMainWindow->connectClose(&MixnMapApp::shutdown, this);
}
void MixnMapApp::shutdown()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		log->logTimedString("shutdown");
		deleteRenderWindows();
		// save warp settings
		mWarpings->save();

		// close ui and save settings
		mSpout->shutdown();
		mTextures->shutdown();
		mUI->shutdown();
	}
}

void MixnMapApp::update()
{
	/*mParameterBag->iChannelTime[0] = getElapsedSeconds();
	mParameterBag->iChannelTime[1] = getElapsedSeconds() - 1;
	mParameterBag->iChannelTime[3] = getElapsedSeconds() - 2;
	mParameterBag->iChannelTime[4] = getElapsedSeconds() - 3;
	//
	if (mParameterBag->mUseTimeWithTempo)
	{
	mParameterBag->iGlobalTime = mParameterBag->iTempoTime*mParameterBag->iTimeFactor;
	}
	else
	{
	mParameterBag->iGlobalTime = getElapsedSeconds();
	}
	mOSC->update();*/
	mUI->update();
	if (mParameterBag->mWindowToCreate > 0)
	{
		// try to create the window only once
		int windowToCreate = mParameterBag->mWindowToCreate;
		mParameterBag->mWindowToCreate = 0;
		switch (windowToCreate)
		{
		case 1:
			createRenderWindow();
			break;
		case 2:
			deleteRenderWindows();
			break;
		}
	}
	mSpout->update();
	updateWindowTitle();
}

void MixnMapApp::drawRender()
{
	//! clear the window and set the drawing color to white
	gl::clear();
	//! draw Spout received textures
	mSpout->draw();
	//TODO? gl::setMatricesWindow(getWindowSize());
	//TODO? gl::setViewport(getWindowBounds());
	mWarpings->draw();
}
void MixnMapApp::drawMain()
{
	mUI->draw();
	gl::disableAlphaBlending();
}
void MixnMapApp::createRenderWindow()
{
	deleteRenderWindows();
	getWindowsResolution();

	mParameterBag->iResolution.x = mParameterBag->mRenderWidth;
	mParameterBag->iResolution.y = mParameterBag->mRenderHeight;
	mParameterBag->mRenderResolution = ivec2(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);

	log->logTimedString("createRenderWindow, resolution:" + toString(mParameterBag->iResolution.x) + "x" + toString(mParameterBag->iResolution.y));

	string windowName = "render";

	WindowRef	mRenderWindow;
	mRenderWindow = createWindow(Window::Format().size(mParameterBag->iResolution.x, mParameterBag->iResolution.y));

	// create instance of the window and store in vector
	WindowMngr rWin = WindowMngr(windowName, mParameterBag->mRenderWidth, mParameterBag->mRenderHeight, mRenderWindow);
	allRenderWindows.push_back(rWin);

#ifdef _DEBUG
	// debug mode
	mRenderWindow->setPos(mParameterBag->mRenderX, 20);	
#else
	mRenderWindow->setBorderless();
	mRenderWindow->setPos(mParameterBag->mRenderX, 0);
#endif  // _DEBUG	
	mParameterBag->mRenderResoXY = vec2(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	mRenderWindow->connectDraw(&MixnMapApp::drawRender, this);
	mParameterBag->mRenderPosXY = ivec2(mParameterBag->mRenderX, 0);

}
void MixnMapApp::deleteRenderWindows()
{
	for (auto wRef : allRenderWindows) DestroyWindow((HWND)wRef.mWRef->getNative());
	allRenderWindows.clear();
}
void MixnMapApp::resize()
{
	mWarpings->resize();

}

void MixnMapApp::mouseMove(MouseEvent event)
{
	mWarpings->mouseMove(event);
}

void MixnMapApp::mouseDown(MouseEvent event)
{
	mWarpings->mouseDown(event);

}

void MixnMapApp::mouseDrag(MouseEvent event)
{
	mWarpings->mouseDrag(event);

}

void MixnMapApp::mouseUp(MouseEvent event)
{
	mWarpings->mouseUp(event);
}

void MixnMapApp::keyDown(KeyEvent event)
{
	mWarpings->keyDown(event);

	// warp editor did not handle the key, so handle it here
	switch (event.getCode())
	{
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;

	case KeyEvent::KEY_m:
		// toggle memoryMode
		mParameterBag->mMemoryMode = !mParameterBag->mMemoryMode;
		break;
	case KeyEvent::KEY_9:
		// toggle dx9
		mParameterBag->mUseDX9 = !mParameterBag->mUseDX9;
		break;
	case KeyEvent::KEY_c:
		// crossfade center
		mParameterBag->controlValues[15] = 0.5;
		break;
	case KeyEvent::KEY_l:
		// crossfade left
		mParameterBag->controlValues[15] = 0.0;
		break;
	case KeyEvent::KEY_r:
		// crossfade right
		mParameterBag->controlValues[15] = 1.0;
		break;
	case KeyEvent::KEY_h:
		hideCursor();
		mParameterBag->mShowUI = false;
		break;
	case KeyEvent::KEY_s:
		showCursor();
		mParameterBag->mShowUI = true;
		break;
	case KeyEvent::KEY_p:
		// flip fbo
		mParameterBag->mFlipFbo = !mParameterBag->mFlipFbo;
		mTextures->flipMixFbo(mParameterBag->mFlipFbo);
		break;
	case KeyEvent::KEY_SPACE:
		// save warp settings
		mWarpings->save();
		break;
	}
}

void MixnMapApp::keyUp(KeyEvent event)
{
	mWarpings->keyUp(event);

}

void MixnMapApp::updateWindowTitle()
{
	if (!mIsShutDown)
	{
		//if (mParameterBag->mShowConsole) { if (getElapsedFrames() % 3000 == 0) log->logTimedString(toString(floor(getAverageFps())) + " fps") };
		getWindow()->setTitle("(" + toString(floor(getAverageFps())) + " fps) Reymenta mix-n-map");
	}
}

CINDER_APP_NATIVE(MixnMapApp, RendererGl)
