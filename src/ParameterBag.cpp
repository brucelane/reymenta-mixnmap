//
//  ParameterBag.cpp
//  From Nathan Selikoff on 4/26/14.
//
//

#include "ParameterBag.h"

using namespace ci;
using namespace std;
using namespace Reymenta;

ParameterBag::ParameterBag()
{
	// reset no matter what, so we don't miss anything
	reset();

	// check to see if ReymentaSettings.xml file exists and restore if it does
	fs::path params = getDocumentsDirectory() / "Reymenta" / settingsFileName;
	if (fs::exists(params))
		restore();
}

ParameterBagRef ParameterBag::create()
{
	return shared_ptr<ParameterBag>(new ParameterBag());
}

bool ParameterBag::save()
{
	// attempt to create "reymenta" directory in Documents directory
	fs::path directory = getDocumentsDirectory() / "Reymenta";
	if (!fs::exists(directory)) {
		if (!createDirectories(directory / settingsFileName)) {
			return false;
		}
	}

	fs::path path = directory / settingsFileName;

	XmlTree settings("settings", "");

	XmlTree DirectRender("ShowConsole", "");
	DirectRender.setAttribute("value", toString(mShowConsole));
	settings.push_back(DirectRender);

	// TODO: test for successful writing of XML
	settings.write(writeFile(path));

	return true;
}

bool ParameterBag::restore()
{
	// check to see if ReymentaSettings.xml file exists
	fs::path params = getDocumentsDirectory() / "Reymenta" / settingsFileName;
	if (fs::exists(params)) {
		// if it does, restore
		const XmlTree xml(loadFile(params));

		if (!xml.hasChild("settings")) {
			return false;
		}
		else {
			const XmlTree settings = xml.getChild("settings");

			if (settings.hasChild("ShowConsole")) {
				XmlTree DirectRender = settings.getChild("ShowConsole");
				mShowConsole = DirectRender.getAttributeValue<bool>("value");
			}

			return true;
		}
	}
	else {
		// if it doesn't, return false
		return false;
	}
}

void ParameterBag::reset()
{
	// spout
	mMemoryMode = false;
	mUseDX9 = false;
	
	mShowUI = true;
	mShowConsole = false;
	// fbo
	mFboWidth = 640;
	mFboHeight = 360;
	mFlipFbo = false;
	// tempo
	mTempo = 166.0;
	mUseTimeWithTempo = false;
	iDeltaTime = 60 / mTempo;
	iTempoTime = 0.0;
	iTimeFactor = 1.0;

	// shader uniforms
	iGlobalTime = 1.0f;
	iResolution = Vec3f(mRenderWidth, mRenderHeight, 1.0);
	for (int i = 0; i < 4; i++)
	{
		iChannelTime[i] = i;
	}
	for (int i = 0; i < 4; i++)
	{
		iChannelResolution[i] = Vec3f(mRenderWidth, mRenderHeight, 1.0);
	}
	iCrossfade = iPreviewCrossfade = 0.5;
	iDebug = iFade = iLight = iLightAuto = iRepeat = false;
	iFps = 60.0;
	iShowFps = true;
	iMouse = Vec4f(mRenderWidth / 2, mRenderHeight / 2, 1.0, 1.0);
	iGreyScale = false;

	// transition
	iTransition = 0;
	iAnim = 0.0;
	mTransitionDuration = 1.0f;

	mOriginUpperLeft = true;
	// OSC
	mOSCDestinationHost = "127.0.0.1";// "192.168.0.18";
	mOSCDestinationPort = 7000;
	mOSCReceiverPort = 9000;
	OSCMsg = "OSC listening on port 9000";
	mRenderWidth = 1024;
	mRenderHeight = 768;
	mRenderXY = mLeftRenderXY = mRightRenderXY = mPreviewRenderXY = Vec2f::zero();
	mRenderPosXY = Vec2f(0.0, 320.0);
	mRenderResoXY = Vec2f(mRenderWidth, mRenderHeight);
	mRenderResolution = Vec2i(mRenderWidth, mRenderHeight);

	currentSelectedIndex = 0;

	for (int a = 0; a < 8; a++)
	{
		iChannels[a] = 0;// a;
	}

	// midi and OSC
	for (int c = 0; c < 128; c++)
	{
		controlValues[c] = 0.01f;
	}

	// red
	controlValues[1] = 1.0f;
	// green
	controlValues[2] = 0.0f;
	// blue
	controlValues[3] = 0.0f;
	// Alpha 
	controlValues[4] = 1.0f;
	// background red
	controlValues[5] = 1.0f;
	// background green
	controlValues[6] = 1.0f;
	// background blue
	controlValues[7] = 0.0f;
	// background alpha
	controlValues[8] = 0.2f;
	// ratio
	controlValues[11] = 20.0f;
	// Speed 
	controlValues[12] = 12.0f;
	// zoom
	controlValues[13] = 1.0f;
	// exposure
	controlValues[14] = 1.0f;
	// crossfade from midi2osc (was Blendmode) 
	controlValues[15] = 0.5f;
	// Steps
	controlValues[16] = 16.0f;
	// Pixelate
	controlValues[18] = 60.0f;
	// RotationSpeed
	controlValues[19] = 1.0f;
	// glitch
	controlValues[45] = 0.0f;
	// toggle
	controlValues[46] = 0.0f;
	// vignette
	controlValues[47] = 0.0f;
	// invert
	controlValues[48] = 0.0f;
}
