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
	settings->setWindowSize(mParameterBag->mRenderWidth/2, mParameterBag->mRenderHeight/2);
	settings->setWindowPos(Vec2i(mParameterBag->mRenderX, mParameterBag->mRenderY + 50));
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
	mParameterBag->mRenderResoXY = Vec2f(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);

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

	// set ui window and io events callbacks
	ImGui::setWindow(getWindow());

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
	Warp::setSize(mWarps, mTextures->getTexture(0).getSize());
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
	ImGui::Shutdown();
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
	gl::setViewport(getWindowBounds());
	int i = 0;
	// iterate over the warps and draw their content
	for (WarpConstIter itr = mWarps.begin(); itr != mWarps.end(); ++itr)
	{
		//log->logTimedString("warp" + toString(i) + " channel:" + toString(mParameterBag->iChannels[i]));
		// create a readable reference to our warp, to prevent code like this: (*itr)->begin();
		WarpRef warp(*itr);

		//warp->draw(mTextures->getMixTexture(mParameterBag->iChannels[i]), mTextures->getMixTexture(mParameterBag->iChannels[i]).getBounds());
		warp->draw(mTextures->getMixTexture(0), mTextures->getMixTexture(0).getBounds());

		i++;
	}
	//imgui
	static float f = 0.0f;

	if (mParameterBag->mShowUI)
	{

		ImGui::NewFrame();

		// start a new window
		ImGui::Begin("mix-n-map parameters", NULL, ImVec2(200, 100));
		{
			// our theme variables
			static float WindowPadding[2] = { 25, 10 };
			static float WindowMinSize[2] = { 160, 80 };
			static float FramePadding[2] = { 4, 4 };
			static float ItemSpacing[2] = { 10, 5 };
			static float ItemInnerSpacing[2] = { 5, 5 };

			static float WindowFillAlphaDefault = 0.7;
			static float WindowRounding = 4;
			static float TreeNodeSpacing = 22;
			static float ColumnsMinSpacing = 50;
			static float ScrollBarWidth = 12;

			if (ImGui::CollapsingHeader("Senders", "0", true, true))
			{
				ImGui::Columns(3, "data", true);
				ImGui::Text("Name"); ImGui::NextColumn();
				ImGui::Text("Width"); ImGui::NextColumn();
				ImGui::Text("Height"); ImGui::NextColumn();
				ImGui::Separator();

				static int w = mSpout->getSenderWidth(0);
				static int h = mSpout->getSenderHeight(0);

				for (int i = 0; i < mSpout->getSenderCount(); i++)
				{
					ImGui::Text(mSpout->getSenderName(i)); ImGui::NextColumn();
					ImGui::SliderInt("w", &w, 0, 1024); ImGui::NextColumn();
					ImGui::SliderInt("h", &h, 0, 1024); ImGui::NextColumn();
				}
				ImGui::Columns(1);
				ImGui::Text("%d senders", mSpout->getSenderCount());

			}
			if (ImGui::CollapsingHeader("Channels", "1", true, true))
			{
				ImGui::Columns(3, "cdata", true);
				ImGui::Text("ActiveSender"); ImGui::NextColumn();
				ImGui::Text("Channel"); ImGui::NextColumn();
				ImGui::Text("Sender"); ImGui::NextColumn();
				ImGui::Separator();

				for (int j = 0; j < mSpout->getSenderCount(); j++)
				{
					ImGui::Text("%d", &j); ImGui::NextColumn();
					ImGui::Text("%d", &mParameterBag->iChannels[j]); ImGui::NextColumn();
					ImGui::Text(mSpout->getSenderName(j)); ImGui::NextColumn();
				}
				ImGui::Columns(1);
			}

			// add a slider to control the background brightness
			ImGui::SliderFloat("Crossfade", &mParameterBag->controlValues[15], 0.0, 1.0);
		}
		ImGui::End();

		ImGui::Render();
	}
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
	if (mParameterBag->mShowConsole) { if (getElapsedFrames() % 3000 == 0) log->logTimedString(toString(floor(getAverageFps())) + " fps") };

	getWindow()->setTitle("(" + toString(floor(getAverageFps())) + " fps) Reymenta mix-n-map");
}

CINDER_APP_BASIC(MixnMapApp, RendererGl)
