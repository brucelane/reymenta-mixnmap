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
	settings->setWindowPos(ivec2(mParameterBag->mRenderX, mParameterBag->mRenderY + 50));
#else
	settings->setWindowSize(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	settings->setWindowPos(Vec2i(mParameterBag->mRenderX, mParameterBag->mRenderY));
#endif  // _DEBUG
	settings->setResizable(true); // allowed for a receiver
	settings->setFrameRate(60.0f);
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
	// instanciate the Shaders class, must not be in prepareSettings
	mShaders = Shaders::create(mParameterBag);
	// instanciate the textures class
	mTextures = Textures::create(mParameterBag, mShaders);
	// instanciate the spout class
	mSpout = SpoutWrapper::create(mParameterBag, mTextures);
	// instanciate the OSC class
	mOSC = OSC::create(mParameterBag);

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

	// adjust the content size of the warps
	Warp::setSize(mWarps, mTextures->getTexture(0)->getSize());
	log->logTimedString("Warps count " + toString(mWarps.size()));

	hideCursor();
	log->logTimedString("setup done");
}

void MixnMapApp::shutdown()
{
	// save warp settings
	log->logTimedString("Saving MixnMapWarps.xml");
	fs::path settings = getAssetPath("") / warpsFileName;
	Warp::writeSettings(mWarps, writeFile(settings));
	// close ui and save settings
	mSpout->shutdown();
	mTextures->shutdown();
}

void MixnMapApp::update()
{
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
	mSpout->update();
	mOSC->update();
	updateWindowTitle();

}

void MixnMapApp::draw()
{
	// clear the window and set the drawing color to white
	gl::clear();
	mSpout->draw();
	gl::setMatricesWindow(getWindowSize());
	//TODO gl::setViewport(getWindowBounds());
	int i = 0;
	// iterate over the warps and draw their content
	for (WarpConstIter itr = mWarps.begin(); itr != mWarps.end(); ++itr)
	{
		//log->logTimedString("warp" + toString(i) + " channel:" + toString(mParameterBag->iChannels[i]));
		// create a readable reference to our warp, to prevent code like this: (*itr)->begin();
		WarpRef warp(*itr);

		//warp->draw(mTextures->getMixTexture(mParameterBag->iChannels[i]), mTextures->getMixTexture(mParameterBag->iChannels[i]).getBounds());
		warp->draw(mTextures->getMixTexture(0), mTextures->getMixTexture(0)->getBounds());

		i++;
	};

}

void MixnMapApp::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize(mWarps);
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
		if (event.isRightDown())
		{ // Select a sender
			//mspoutreceiver.SelectSenderPanel(); // SpoutPanel.exe must be in the executable path

		}
	}
}

void MixnMapApp::mouseDrag(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDrag(mWarps, event))
	{
		// let your application perform its mouseDrag handling here
	}
}

void MixnMapApp::mouseUp(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseUp(mWarps, event))
	{

	}
}

void MixnMapApp::keyDown(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyDown(mWarps, event))
	{
		// warp editor did not handle the key, so handle it here
		switch (event.getCode())
		{
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_f:
			// toggle full screen
			setFullScreen(!isFullScreen());
			break;
		case KeyEvent::KEY_w:
			// toggle warp edit mode
			Warp::enableEditMode(!Warp::isEditModeEnabled());
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
			Warp::writeSettings(mWarps, writeFile(getAssetPath("") / warpsFileName));
			break;
		case KeyEvent::KEY_n:
			mWarps.push_back(WarpPerspectiveBilinear::create());
			break;
		}
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
	//if (mParameterBag->mShowConsole) { if (getElapsedFrames() % 3000 == 0) log->logTimedString(toString(floor(getAverageFps())) + " fps") };

	getWindow()->setTitle("(" + toString(floor(getAverageFps())) + " fps) Reymenta mix-n-map");
}

CINDER_APP_NATIVE(MixnMapApp, RendererGl)
