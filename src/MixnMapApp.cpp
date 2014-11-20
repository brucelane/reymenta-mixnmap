#include "MixnMapApp.h"

void MixnMapApp::prepareSettings(Settings *settings)
{
	// instanciate the logger class
	log = Logger::create("MixnMap.txt");
	log->logTimedString("start");
	mLogMsg = "";
	// parameters
	mParameterBag = ParameterBag::create();
	getWindowsResolution();
#ifdef _DEBUG
	// debug mode
	settings->setWindowSize(mParameterBag->mMainDisplayWidth/2, mParameterBag->mMainDisplayHeight - 200);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX - mParameterBag->mMainDisplayWidth/1.9, mParameterBag->mRenderY + 50));
#else
	settings->setWindowSize(mParameterBag->mMainDisplayWidth, mParameterBag->mMainDisplayHeight);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX - mParameterBag->mMainDisplayWidth, mParameterBag->mRenderY));
	setBorderless();
#endif  // _DEBUG
	settings->setResizable(true); // allowed for a receiver
	// set a high frame rate to disable limitation
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
	mParameterBag->mMainDisplayHeight = Display::getMainDisplay()->getHeight();
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
	// instanciate the audio class
	mAudio = AudioWrapper::create(mParameterBag, mTextures);
	windowManagement();
	// Setup the MinimalUI user interface
	mUI = UI::create(mParameterBag, mShaders, mTextures, mMainWindow);
	mUI->setup();
	// instanciate the warp wrapper class
	//mWarpings = WarpWrapper::create(mParameterBag, mTextures, mShaders);

	// initialize warps
	log->logTimedString("Loading MixnMapWarps.xml");
	fs::path settings = getAssetPath("") / warpsFileName;
	if (fs::exists(settings))
	{
		// load warp settings from file if one exists
		mWarps = Warp::readSettings(loadFile(settings));
	}
	else
	{
		// otherwise create a warp from scratch
		mWarps.push_back(WarpPerspectiveBilinear::create());
	}
	mSrcArea = mTextures->getTexture(0)->getBounds();
	// adjust the content size of the warps
	Warp::setSize(mWarps, mTextures->getTexture(0)->getSize());
	log->logTimedString("Warps count " + toString(mWarps.size()));

	gl::enableDepthRead();
	gl::enableDepthWrite();
	mTimer = 0.0f;
	mUI->tapTempo(true);

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
	log->logTimedString("shutdown");
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		log->logTimedString("1st screen shutdown must be done once");
		deleteRenderWindows();
		// save warp settings
		//mWarpings->save();
		fs::path settings = getAssetPath("") / warpsFileName;
		Warp::writeSettings(mWarps, writeFile(settings));

		// close ui and save settings
		mSpout->shutdown();
		mTextures->shutdown();
		mUI->shutdown();
		timeline().apply(&mTimer, 1.0f, 1.0f).finishFn([&]{ quit(); });
	}

}
void MixnMapApp::fileDrop(FileDropEvent event)
{
	string ext = "";
	// use the last of the dropped files
	boost::filesystem::path mPath = event.getFile(event.getNumFiles() - 1);
	string mFile = mPath.string();
	if (mFile.find_last_of(".") != std::string::npos) ext = mFile.substr(mFile.find_last_of(".") + 1);
	//mParameterBag->currentSelectedIndex = (int)(event.getX() / 80);//76+margin mParameterBag->mPreviewWidth);
	log->logTimedString(mFile + " dropped, currentSelectedIndex:" + toString(mParameterBag->currentSelectedIndex) + " x: " + toString(event.getX()) + " mPreviewWidth: " + toString(mParameterBag->mPreviewWidth));

	if (ext == "wav" || ext == "mp3")
	{
		mAudio->loadWaveFile(mFile);
	}
	else if (ext == "png" || ext == "jpg")
	{
		//mTextures->loadImageFile(mParameterBag->currentSelectedIndex, mFile);
		mTextures->setTextureFromFile(1, mFile);
	}
	else if (ext == "glsl")
	{
		//mUserInterface->mLibraryPanel->addShader(mFile);
		if (mShaders->loadPixelFragmentShader(mFile))
		{
			mParameterBag->controlValues[13] = 1.0f;
			timeline().apply(&mTimer, 1.0f, 1.0f).finishFn([&]{ mTextures->saveThumb(); });
		}
	}

}

void MixnMapApp::update()
{
	if (mParameterBag->iGreyScale)
	{
		mParameterBag->controlValues[1] = mParameterBag->controlValues[2] = mParameterBag->controlValues[3];
		mParameterBag->controlValues[5] = mParameterBag->controlValues[6] = mParameterBag->controlValues[7];
	}
	mParameterBag->iChannelTime[0] = getElapsedSeconds();
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
	mOSC->update();
	mAudio->update();
	//! update textures
	mTextures->update();
	//! update shaders (must be after the textures update)
	mShaders->update();
	if (mParameterBag->mShowUI) mUI->update();
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
	//! clear the window
	gl::clear();
	gl::pushMatrices();
	gl::setMatricesWindow(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	gl::pushViewport(0, 0, mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	gl::enableAlphaBlending();
	//mWarpings->draw();
	// iterate over the warps and draw their content
	int i = 0;
	for (auto &warp : mWarps) 
	{
		//warp->draw(mTextures->getTexture(i), mSrcArea);
		warp->draw(mTextures->getTexture(i), mTextures->getTexture(i)->getBounds());
		i++;
	}

	gl::disableAlphaBlending();
	gl::popViewport();
	gl::popMatrices();
}
void MixnMapApp::drawMain()
{
	//! clear the window
	gl::clear(ColorAf(0.0f, 0.0f, 0.0f, 0.0f));
	//! draw Spout received textures
	mSpout->draw();
	mTextures->draw();
	if (mParameterBag->mShowUI) mUI->draw();
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
	mShaders->resize();
	//mWarpings->resize();
}

void MixnMapApp::mouseMove(MouseEvent event)
{
	//mWarpings->mouseMove(event);
}

void MixnMapApp::mouseDown(MouseEvent event)
{
	mParameterBag->iMouse.z = event.getX();
	mParameterBag->iMouse.w = getWindowHeight() - event.getY();
}

void MixnMapApp::mouseDrag(MouseEvent event)
{
	mParameterBag->iMouse.x =  event.getX();
	mParameterBag->iMouse.y = getWindowHeight() - event.getY();
	//mWarpings->mouseDrag(event);
}

void MixnMapApp::mouseUp(MouseEvent event)
{
	//mWarpings->mouseUp(event);
}

void MixnMapApp::keyDown(KeyEvent event)
{
	//mWarpings->keyDown(event);

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
		//mWarpings->save();
		break;
	}
}

void MixnMapApp::keyUp(KeyEvent event)
{
	//mWarpings->keyUp(event);

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
