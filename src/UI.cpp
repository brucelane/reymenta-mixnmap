#include "UI.h"

using namespace Reymenta;
/*
	4:3 w h
	btn: 48 36
	tex: 76 57
	pvw: 156 88
	*/
UI::UI(ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, WindowRef aWindow, OSCRef aOscRef)
{
	mParameterBag = aParameterBag;
	mShaders = aShadersRef;
	mTextures = aTexturesRef;
	mWindow = aWindow;
	mOSC = aOscRef;

	mCbMouseDown = mWindow->getSignalMouseDown().connect(0, std::bind(&UI::mouseDown, this, std::placeholders::_1));
	mCbKeyDown = mWindow->getSignalKeyDown().connect(0, std::bind(&UI::keyDown, this, std::placeholders::_1));

	mVisible = true;
	mSetupComplete = false;
	// tempo
	mParameterBag->iDeltaTime = 60 / mParameterBag->mTempo;
	mSpaghetti = Spaghetti::create(mParameterBag, mTextures);
}

UIRef UI::create(ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, app::WindowRef aWindow, OSCRef aOscRef)
{
	return shared_ptr<UI>(new UI(aParameterBag, aShadersRef, aTexturesRef, aWindow, aOscRef));
}

void UI::setup()
{
	// load custom fonts (I do this once, in the UI class)
	// UI fonts
	mParameterBag->mLabelFont = Font(loadResource(RES_HELVETICA_NEUE_REGULAR), 14 * 2);
	mParameterBag->mSmallLabelFont = Font(loadResource(RES_HELVETICA_NEUE_REGULAR), 12 * 2);
	mParameterBag->mHeaderFont = Font(loadResource(RES_HELVETICA), 24 * 2);
	mParameterBag->mBodyFont = Font(loadResource(RES_GARAMOND), 19 * 2);
	mParameterBag->mFooterFont = Font(loadResource(RES_GARAMOND_ITALIC), 14 * 2);

	setupMiniControl();
	setupShaders();
	setupTextures();
	setupLibrary();
	setupWarps();

	mSetupComplete = true;
}
void UI::createWarp()
{
	mTextures->createWarpInput();
	addMixControls();
}

void UI::setupMiniControl()
{
	mMiniControl = UIController::create("{ \"x\":0, \"y\":0, \"depth\":100, \"width\":960, \"height\":144, \"fboNumSamples\":0, \"panelColor\":\"0x44402828\" }");
	mMiniControl->DEFAULT_UPDATE_FREQUENCY = mParameterBag->mUIRefresh;
	mMiniControl->setFont("label", mParameterBag->mLabelFont);
	mMiniControl->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	mMiniControl->setFont("icon", mParameterBag->mIconFont);
	mMiniControl->setFont("header", mParameterBag->mHeaderFont);
	mMiniControl->setFont("body", mParameterBag->mBodyFont);
	mMiniControl->setFont("footer", mParameterBag->mFooterFont);
	mPanels.push_back(mMiniControl);

	fpsMvg = mMiniControl->addMovingGraphButton("fps", &mParameterBag->iFps, std::bind(&UI::showFps, this, std::placeholders::_1), "{ \"clear\":false, \"stateless\":false, \"width\":76, \"min\":0.0, \"max\":120.0 }");

	// output 
	mMiniControl->addLabel("Rndr\nwin", "{ \"width\":48, \"clear\":false }");
	mMiniControl->addButton("1", std::bind(&UI::createRenderWindow, this, 1, std::placeholders::_1), "{ \"clear\":false, \"stateless\":false, \"group\":\"render\", \"exclusive\":true }");
	mMiniControl->addButton("x", std::bind(&UI::deleteRenderWindows, this, std::placeholders::_1), "{ \"clear\":false, \"stateless\":false, \"group\":\"render\", \"exclusive\":true, \"pressed\":true }");

	mMiniControl->addButton("OSC\nRcv", std::bind(&UI::setupOSCReceiver, this, std::placeholders::_1), "{ }");
	labelOSC = mMiniControl->addLabel("OSC", "{ \"width\":960 }");
	labelError = mMiniControl->addLabel("", "{ \"width\":960 }");
}
void UI::setupWarps()
{
	wParams = UIController::create("{ \"visible\":true, \"x\":766, \"y\":150, \"width\":140, \"height\":530, \"depth\":207, \"panelColor\":\"0x44284828\" }", mWindow);
	wParams->DEFAULT_UPDATE_FREQUENCY = mParameterBag->mUIRefresh;
	wParams->setFont("label", mParameterBag->mLabelFont);
	wParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	wParams->setFont("icon", mParameterBag->mIconFont);
	wParams->setFont("header", mParameterBag->mHeaderFont);
	wParams->setFont("body", mParameterBag->mBodyFont);
	wParams->setFont("footer", mParameterBag->mFooterFont);
	mPanels.push_back(wParams);
	// warp index
	currentIndexLabel = wParams->addLabel("Select\nWarp", "{ \"clear\":false, \"width\":48 }");
	wParams->addButton("+", std::bind(&UI::setCurrentFboIndex, this, 1, std::placeholders::_1), "{ \"clear\":false, \"width\":36 }");
	wParams->addButton("-", std::bind(&UI::setCurrentFboIndex, this, -1, std::placeholders::_1), "{  \"width\":36 }");

}
void UI::addButtons()
{
	int i = buttonIndex.size();
	buttonIndex.push_back(wParams->addButton(toString(i), std::bind(&UI::setCurrentIndex, this, i, std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":false, \"group\":\"index\", \"exclusive\":true }"));
	labelFboIndex.push_back(wParams->addLabel(toString(mParameterBag->iWarpFboChannels[i]), "{ \"width\":48 }"));
}
void UI::setCurrentIndex(const int &aIndex, const bool &pressed)
{
	if (pressed)
	{
		warpIndex = aIndex;
		mOSC->sendOSCMessage("/select", aIndex,0,0,0);
		currentIndexLabel->setName("Warp\n" + toString(aIndex));
	}
}
void UI::setCurrentFboIndex(const int &aIndex, const bool &pressed)
{
	mParameterBag->iWarpFboChannels[warpIndex] += aIndex;
	if (mParameterBag->iWarpFboChannels[warpIndex] > 7) mParameterBag->iWarpFboChannels[warpIndex] = 0;
	if (mParameterBag->iWarpFboChannels[warpIndex] < 0) mParameterBag->iWarpFboChannels[warpIndex] = 7;
	labelFboIndex[warpIndex]->setName(toString(mParameterBag->iWarpFboChannels[warpIndex]));
	mOSC->sendOSCMessage("/channel", mParameterBag->iWarpFboChannels[warpIndex], 0, 0, 0);
}
void UI::setupTextures()
{
	tParams = UIController::create("{ \"x\":0, \"y\":150, \"depth\":300, \"width\":176, \"height\":530, \"marginLarge\":2, \"fboNumSamples\":0, \"panelColor\":\"0x44282828\", \"defaultBackgroundColor\":\"0xFF0d0d0d\", \"defaultNameColor\":\"0xFF90a5b6\", \"defaultStrokeColor\":\"0xFF282828\", \"activeStrokeColor\":\"0xFF919ea7\" }", mWindow);
	tParams->DEFAULT_UPDATE_FREQUENCY = mParameterBag->mUIRefresh;
	tParams->setFont("label", mParameterBag->mLabelFont);
	tParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	tParams->setFont("icon", mParameterBag->mIconFont);
	tParams->setFont("header", mParameterBag->mHeaderFont);
	tParams->setFont("body", mParameterBag->mBodyFont);
	tParams->setFont("footer", mParameterBag->mFooterFont);
	mPanels.push_back(tParams);
	tParams->addLabel("Input textures", "{ \"width\":100 }");
}
void UI::addTextureControls()
{
	// Textures select
	// Button Group: textures
	buttonTexture.push_back(tParams->addButton(toString(buttonTexture.size()), std::bind(&UI::setTextureIndex, this, buttonTexture.size(), std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":true, \"group\":\"textures\", \"exclusive\":true }"));
	labelTexture.push_back(tParams->addLabel(toString(labelTexture.size()), "{ \"width\":100 }"));
}
void UI::setupShaders()
{
	sParams = UIController::create("{ \"x\":178, \"y\":150, \"depth\":300, \"width\":176, \"height\":530, \"marginLarge\":2, \"fboNumSamples\":0, \"panelColor\":\"0x44282828\", \"defaultBackgroundColor\":\"0xFF0d0d0d\", \"defaultNameColor\":\"0xFF90a5b6\", \"defaultStrokeColor\":\"0xFF282828\", \"activeStrokeColor\":\"0xFF919ea7\" }", mWindow);
	sParams->DEFAULT_UPDATE_FREQUENCY = mParameterBag->mUIRefresh;
	sParams->setFont("label", mParameterBag->mLabelFont);
	sParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	sParams->setFont("icon", mParameterBag->mIconFont);
	sParams->setFont("header", mParameterBag->mHeaderFont);
	sParams->setFont("body", mParameterBag->mBodyFont);
	sParams->setFont("footer", mParameterBag->mFooterFont);
	mPanels.push_back(sParams);
	sParams->addLabel("Shaders", "{ \"width\":100 }");
}
void UI::setupLibrary()
{
	mixParams = UIController::create("{ \"visible\":true, \"x\":356, \"y\":150, \"width\":400, \"height\":530, \"depth\":203, \"panelColor\":\"0x44482828\" }");
	mixParams->DEFAULT_UPDATE_FREQUENCY = mParameterBag->mUIRefresh;
	mixParams->setFont("label", mParameterBag->mLabelFont);
	mixParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	mixParams->setFont("icon", mParameterBag->mIconFont);
	mixParams->setFont("header", mParameterBag->mHeaderFont);
	mixParams->setFont("body", mParameterBag->mBodyFont);
	mixParams->setFont("footer", mParameterBag->mFooterFont);
	mPanels.push_back(mixParams);
	mixParams->addLabel("Texture mixing", "{  }");
}

void UI::setPreview(const int &aIndex, const bool &pressed)
{

}
void UI::addMixControls()
{
	int i = buttonLeft.size();
	iCrossfade.push_back(0.5);
	buttonLeft.push_back(mixParams->addButton("L", std::bind(&UI::setLeftInput, this, i, std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":true, \"group\":\"c0\", \"exclusive\":true }"));
	buttonRight.push_back(mixParams->addButton("R", std::bind(&UI::setRightInput, this, i, std::placeholders::_1), "{  \"clear\":false, \"width\":48, \"stateless\":true, \"group\":\"c1\", \"exclusive\":true }"));
	buttonSelect.push_back(mixParams->addButton(toString(i), std::bind(&UI::setPreview, this, i, std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":false, \"group\":\"pvw\", \"exclusive\":true }"));
	sliderCrossfade.push_back(mixParams->addSlider("xFade", &iCrossfade[i], "{ \"min\":0.0, \"max\":1.0, \"width\":96 }"));
}
void UI::addShadaControls()
{
	// Shaders select
	// Button Group: shaders
	buttonShada.push_back(sParams->addButton(toString(buttonShada.size()), std::bind(&UI::setShadaIndex, this, buttonShada.size(), std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":true, \"group\":\"shaders\", \"exclusive\":true }"));
	labelShada.push_back(sParams->addLabel(toString(labelShada.size()), "{ \"width\":100 }"));
}

void UI::setLeftInput(const int &aIndex, const bool &pressed)
{
	WarpInput wi = mTextures->setInput(aIndex, true);
	buttonLeft[aIndex]->setName(toString(wi.leftIndex));
	if (wi.leftMode == 0)
	{
		buttonLeft[aIndex]->setBackgroundTexture(mTextures->getTexture(wi.leftIndex));
	}
	else
	{
		buttonLeft[aIndex]->setBackgroundTexture(mTextures->getFboTexture(wi.leftIndex));
	}
	mOSC->sendOSCMessage("/texture", aIndex, true, wi.leftMode, wi.leftIndex);
	buttonSelect[aIndex]->setBackgroundTexture(mTextures->getMixTexture(aIndex));
	// add path
	mSpaghetti->drawPath();
}
void UI::setRightInput(const int &aIndex, const bool &pressed)
{
	WarpInput wi = mTextures->setInput(aIndex, false);
	buttonRight[aIndex]->setName(toString(wi.rightIndex));
	if (wi.rightMode == 0)
	{
		buttonRight[aIndex]->setBackgroundTexture(mTextures->getTexture(wi.rightIndex));
	}
	else
	{
		buttonRight[aIndex]->setBackgroundTexture(mTextures->getFboTexture(wi.rightIndex));
	}
	mOSC->sendOSCMessage("/texture", aIndex, false, wi.rightMode, wi.rightIndex);
	buttonSelect[aIndex]->setBackgroundTexture(mTextures->getMixTexture(aIndex));
	// add path
	mSpaghetti->drawPath();
}
void UI::setTextureIndex(const int &aTextureIndex, const bool &pressed)
{
	mTextures->setInputTextureIndex(aTextureIndex);
	mParameterBag->iChannels[0] = aTextureIndex;

	// add path
	mSpaghetti->drawPath();
}
void UI::setShadaIndex(const int &aShadaIndex, const bool &pressed)
{
	mParameterBag->mCurrentShadaFboIndex = aShadaIndex;
	mTextures->setShadaIndex(aShadaIndex);
}

void UI::mouseDown(MouseEvent event)
{
	mSpaghetti->mouseDown(event);
}

void UI::mouseUp(MouseEvent event)
{
	mSpaghetti->mouseUp(event);
}
void UI::mouseDrag(MouseEvent event)
{
	mSpaghetti->mouseDrag(event);
}
void UI::keyDown(KeyEvent event)
{
	
}

void UI::draw()
{
	// normal alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (mVisible)
	{
		for (auto & panel : mPanels) panel->draw();
	}
	// draw the control points
	mSpaghetti->draw();
}
string UI::formatNumber(float f)
{
	f *= 100;
	f = (float)((int)f);
	return toString(f);
}
void UI::update()
{
	if (mVisible)
	{
		for (int i = 0; i < buttonLeft.size(); i++)	mTextures->warpInputs[i].iCrossfade = iCrossfade[i];

		for (auto & panel : mPanels) panel->update();

		if (mParameterBag->iFps > 12.0)
		{
			mParameterBag->FPSColor = UIController::DEFAULT_NAME_COLOR;
		}
		else
		{
			mParameterBag->FPSColor = mParameterBag->ColorRed;
		}
		fpsMvg->setNameColor(mParameterBag->FPSColor);


		if (mParameterBag->controlValues[12] == 0.0) mParameterBag->controlValues[12] = 0.01;

		// fps
		fpsMvg->setName(toString(floor(mParameterBag->iFps)) + " fps");

		labelOSC->setName(mParameterBag->OSCMsg);
		labelError->setName(mShaders->getFragError());

		for (int i = 0; i < buttonShada.size(); i++)
		{
			buttonShada[i]->setBackgroundTexture(mTextures->getFboTexture(i));
			labelShada[i]->setName(mShaders->getShaderName(i));
		}
		for (int i = 0; i < buttonTexture.size(); i++)
		{
			buttonTexture[i]->setBackgroundTexture(mTextures->getTexture(i));
			labelTexture[i]->setName(mTextures->getSenderName(i));
		}
		for (int i = 0; i < buttonIndex.size(); i++)
		{
			buttonIndex[i]->setBackgroundTexture(mTextures->getMixTexture(mParameterBag->iWarpFboChannels[i]));
		}
	}
}
void UI::resize()
{
	for (auto & panel : mPanels) panel->resize();

}

void UI::show()
{
	mVisible = true;
	AppBasic::get()->showCursor();
}

void UI::hide()
{
	mVisible = false;
}

void UI::toggleUseTimeWithTempo(const bool &pressed)
{
	mParameterBag->mUseTimeWithTempo = pressed;
}

// show Fps
void UI::showFps(const bool &pressed)
{
	mParameterBag->iShowFps = pressed;
}
void UI::saveSettings(const bool &pressed)
{
	if (mParameterBag->save()) {
	}
	else
	{
	}
}

void UI::restoreSettings(const bool &pressed)
{
	if (mParameterBag->restore()) {
	}
	else
	{
	}
}

void UI::resetSettings(const bool &pressed)
{
	mParameterBag->reset();
}
void UI::shutdown()
{
}