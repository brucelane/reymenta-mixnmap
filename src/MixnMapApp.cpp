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
	settings->setWindowSize(mParameterBag->mMainDisplayWidth - 300, mParameterBag->mMainDisplayHeight - 200);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX - mParameterBag->mMainDisplayWidth + 250, mParameterBag->mRenderY + 50));
#else
	settings->setWindowSize(mParameterBag->mMainDisplayWidth*2/3, mParameterBag->mMainDisplayHeight - 200);
	settings->setWindowPos(ivec2(mParameterBag->mRenderX - mParameterBag->mMainDisplayWidth, mParameterBag->mRenderY + 50));
	//setBorderless();
#endif  // _DEBUG
	settings->setResizable(true); // allowed for a receiver
	// set a high frame rate to disable limitation
	settings->setFrameRate(1000.0f);
	console() << "MT: " << System::hasMultiTouch() << " Max points: " << System::getMaxMultiTouchPoints() << std::endl;
	if (System::hasMultiTouch())
	{
		settings->enableMultiTouch(); 
		mParameterBag->mMultiTouchEnabled = true;
	}
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
	// instanciate the OSC class
	mOSC = OSC::create(mParameterBag);
	// instanciate the Shaders class, must not be in prepareSettings
	mShaders = Shaders::create(mParameterBag, mOSC);
	// instanciate the textures class
	mTextures = Textures::create(mParameterBag, mShaders);
	// instanciate the spout class
	mSpout = SpoutWrapper::create(mParameterBag, mTextures);
	// instanciate the audio class
	mAudio = AudioWrapper::create(mParameterBag, mTextures);
	windowManagement();
	// Setup the MinimalUI user interface
	mUI = UI::create(mParameterBag, mShaders, mTextures, mMainWindow, mOSC);
	mUI->setup();

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
	// create warpInputs and UI
	for (auto &warp : mWarps)
	{
		mUI->createWarp();
	}
	mOSC->sendOSCMessage("/createwarps", mWarps.size(), 0, 0, 0);
	setSelectedWarp(0);
	// adjust the content size of the warps
	Warp::setSize(mWarps, ivec2(mParameterBag->mFboWidth, mParameterBag->mFboHeight));//mTextures->getTexture(0)->getSize());
	log->logTimedString("Warps count " + toString(mWarps.size()));

	gl::enableDepthRead();
	gl::enableDepthWrite();
	mTimer = 0.0f;

#ifdef _DEBUG
	// debug mode
#else
	hideCursor();
#endif  // _DEBUG	
	log->logTimedString("setup done");
}
void MixnMapApp::setSelectedWarp(int index) 
{ 
	int selectedWarp = min((int)mWarps.size(), index); 
	// set warpIndex and send OSC msg
	mUI->setCurrentIndex(selectedWarp, true);
}

void MixnMapApp::windowManagement()
{
	log->logTimedString("windowManagement");
	getWindowsResolution();
	// setup the main window and associated draw function
	mMainWindow = getWindow();
	mMainWindow->setTitle("Reymenta mix-n-map");
	mainDrawConnection = mMainWindow->connectDraw(&MixnMapApp::drawMain, this);
	mMainWindow->connectClose(&MixnMapApp::shutdown, this);
}
void MixnMapApp::shutdown()
{
	log->logTimedString("shutdown");
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		log->logTimedString("1st screen shutdown must be done once");
		renderDrawConnection.disconnect();
		mainDrawConnection.disconnect();
		deleteRenderWindows();
		// save warp settings
		fs::path settings = getAssetPath("") / warpsFileName;
		Warp::writeSettings(mWarps, writeFile(settings));
		// save params
		mParameterBag->save();

		// close ui and save settings
		mSpout->shutdown();
		mTextures->shutdown();
		mUI->shutdown();
		// TODO causes error in vao.h, and it never quits if this is not called.. timeline().apply(&mTimer, 1.0f, 1.0f).finishFn([&]{ quit(); });
	}

}
void MixnMapApp::fileDrop(FileDropEvent event)
{
	string ext = "";
	// use the last of the dropped files
	boost::filesystem::path mPath = event.getFile(event.getNumFiles() - 1);
	string mFile = mPath.string();
	if (mFile.find_last_of(".") != std::string::npos) ext = mFile.substr(mFile.find_last_of(".") + 1);
	// transform to lower case
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	//mParameterBag->selectedInputTexture = (int)(event.getX() / 80);//76+margin mParameterBag->mPreviewWidth);
	log->logTimedString(mFile + " dropped, x: " + toString(event.getX()) + " mPreviewWidth: " + toString(mParameterBag->mPreviewWidth));

	if (ext == "wav" || ext == "mp3")
	{
		mAudio->loadWaveFile(mFile);
	}
	else if (ext == "png" || ext == "jpg")
	{
		mTextures->setTextureFromFile(mFile);
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
	if (!mIsShutDown)
	{
		// compare then number of shaders to the number of ShadaFbos
		// in case of a new shader, add a ShadaFbo TODO: recycle if size is > MAX
		if (mTextures->getShadaFbosSize() < mShaders->getShaderCount())
		{
			mTextures->addShadaFbo();
			mUI->addShadaControls();
		}
		// compare then number of warps to the number of UI warpPanel buttons
		// in case of a new warp, add a line
		int warpsSize = 0;
		for (auto &warp : mWarps)
		{
			warpsSize++;
		}
		if (mUI->getWarpsSize() < warpsSize)
		{
			mUI->addButtons();
		}
		// compare then number of textures to the number of ui tParams elements
		// in case of a new texture, create the new uiElements
		if (mUI->getTextureButtonsCount() < mTextures->getInputTexturesCount())
		{
			mUI->addTextureControls();
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
}

void MixnMapApp::drawRender()
{
	//! clear the window
	gl::clear();
	gl::pushMatrices();
	// origin upper left set to false for warps
	gl::setMatricesWindow(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight, false);
	gl::pushViewport(0, 0, mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);

	gl::enableAlphaBlending();
	// iterate over the warps and draw their content
	int i = 0;
	for (auto &warp : mWarps)
	{
		warp->draw(mTextures->getMixTexture(mParameterBag->iWarpFboChannels[i]), mTextures->getMixTexture(mParameterBag->iWarpFboChannels[i])->getBounds());
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
	//! draw current shader in the background	
	//gl::pushMatrices();
	gl::setMatricesWindow(mParameterBag->mFboWidth, mParameterBag->mFboHeight);
	gl::draw(mTextures->getFboTexture(mParameterBag->mCurrentShadaFboIndex));
	//gl::popViewport();
	//gl::popMatrices();
	//! draw UI
	if (mParameterBag->mShowUI) mUI->draw();

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
	mRenderWindow->setPos(mParameterBag->mRenderX, 40);	
#else
	mRenderWindow->setBorderless();
	mRenderWindow->setPos(mParameterBag->mRenderX, 0);
#endif  // _DEBUG	
	mParameterBag->mRenderResoXY = vec2(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	renderDrawConnection = mRenderWindow->connectDraw(&MixnMapApp::drawRender, this);
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
}

void MixnMapApp::mouseMove(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseMove(mWarps, event))
	{
		// let your application perform its mouseMove handling here
	}
}

void MixnMapApp::mouseDown(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDown(mWarps, event))
	{
		// let your application perform its mouseDown handling here
		mUI->mouseDown(event);
	}
	mParameterBag->iMouse.z = event.getX();
	mParameterBag->iMouse.w = getWindowHeight() - event.getY();
}

void MixnMapApp::mouseDrag(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDrag(mWarps, event))
	{
		// let your application perform its mouseDrag handling here
		mUI->mouseDrag(event);
	}
	mParameterBag->iMouse.x = event.getX();
	mParameterBag->iMouse.y = getWindowHeight() - event.getY();
}

void MixnMapApp::mouseUp(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseUp(mWarps, event))
	{
		// let your application perform its mouseUp handling here
		mUI->mouseUp(event);
	}
}

void MixnMapApp::keyDown(KeyEvent event)
{
	// warp editor did not handle the key, so handle it here
	switch (event.getCode())
	{
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;
	case KeyEvent::KEY_n:
		// create a warp
		mWarps.push_back(WarpPerspectiveBilinear::create());
		break;
	case KeyEvent::KEY_w:
		// toggle warp edit mode
		Warp::enableEditMode(!Warp::isEditModeEnabled());
		break;
	case KeyEvent::KEY_0:
		// select warp
		setSelectedWarp(0);
		break;
	case KeyEvent::KEY_1:
		// select warp
		setSelectedWarp(1);
		break;
	case KeyEvent::KEY_2:
		// select warp
		setSelectedWarp(2);
		break;
	case KeyEvent::KEY_3:
		// select warp
		setSelectedWarp(3);
		break;
	case KeyEvent::KEY_4:
		// select warp
		setSelectedWarp(4);
		break;
	case KeyEvent::KEY_5:
		// select warp
		setSelectedWarp(5);
		break;
	case KeyEvent::KEY_6:
		// select warp
		setSelectedWarp(6);
		break;
	case KeyEvent::KEY_7:
		// select warp
		setSelectedWarp(7);
		break;
	case KeyEvent::KEY_8:
		// select warp
		setSelectedWarp(8);
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
	case KeyEvent::KEY_SPACE:
		// save warp settings
		fs::path settings = getAssetPath("") / warpsFileName;
		Warp::writeSettings(mWarps, writeFile(settings));
		break;
	}
}

void MixnMapApp::keyUp(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyUp(mWarps, event))
	{
		// let your application perform its keyUp handling here
	}
}

void MixnMapApp::updateWindowTitle()
{
	mParameterBag->iFps = getAverageFps();
	getWindow()->setTitle("(" + toString(floor(mParameterBag->iFps)) + " fps) Reymenta mix-n-map");
}

CINDER_APP_NATIVE(MixnMapApp, RendererGl)
