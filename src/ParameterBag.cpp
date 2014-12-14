/**
* \file ParameterBag.cpp
* \author Bruce LANE/Nathan Selikoff
* \date 20 november 2014
*
* Parameters for all classes.
*
*/

#include "ParameterBag.h"

using namespace ci;
using namespace std;
using namespace Reymenta;

ParameterBag::ParameterBag()
{
	// reset no matter what, so we don't miss anything
	reset();

	// check to see if ReymentaSettings.xml file exists and restore if it does
	fs::path params = getAssetPath("") / settingsFileName;
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
	fs::path directory = getAssetPath("");
	if (!fs::exists(directory)) {
		if (!createDirectories(directory / settingsFileName)) {
			return false;
		}
	}

	fs::path path = directory / settingsFileName;

	XmlTree settings("settings", "");

	XmlTree ShowConsole("ShowConsole", "");
	ShowConsole.setAttribute("value", toString(mShowConsole));
	settings.push_back(ShowConsole);

	XmlTree UseDX9("UseDX9", "");
	UseDX9.setAttribute("value", toString(mUseDX9));
	settings.push_back(UseDX9);

	XmlTree ShowUI("ShowUI", "");
	ShowUI.setAttribute("value", toString(mShowUI));
	settings.push_back(ShowUI);

	XmlTree FboWidth("FboWidth", "");
	FboWidth.setAttribute("value", toString(mFboWidth));
	settings.push_back(FboWidth);

	XmlTree FboHeight("FboHeight", "");
	FboHeight.setAttribute("value", toString(mFboHeight));
	settings.push_back(FboHeight);

	XmlTree OSCReceiverPort("OSCReceiverPort", "");
	OSCReceiverPort.setAttribute("value", toString(mOSCReceiverPort));
	settings.push_back(OSCReceiverPort);

	XmlTree OSCDestinationPort("OSCDestinationPort", "");
	OSCDestinationPort.setAttribute("value", toString(mOSCDestinationPort));
	settings.push_back(OSCDestinationPort);

	XmlTree OSCDestinationHost("OSCDestinationHost", "");
	OSCDestinationHost.setAttribute("value", toString(mOSCDestinationHost));
	settings.push_back(OSCDestinationHost);

	// TODO: test for successful writing of XML
	settings.write(writeFile(path));

	return true;
}

bool ParameterBag::restore()
{
	// check to see if ReymentaSettings.xml file exists
	fs::path params = getAssetPath("") / settingsFileName;
	if (fs::exists(params)) {
		// if it does, restore
		const XmlTree xml(loadFile(params));

		if (!xml.hasChild("settings")) {
			return false;
		}
		else {
			const XmlTree settings = xml.getChild("settings");

			if (settings.hasChild("ShowConsole")) {
				XmlTree ShowConsole = settings.getChild("ShowConsole");
				mShowConsole = ShowConsole.getAttributeValue<bool>("value");
			}
			if (settings.hasChild("UseDX9")) {
				XmlTree UseDX9 = settings.getChild("UseDX9");
				mUseDX9 = UseDX9.getAttributeValue<bool>("value");
			}
			if (settings.hasChild("ShowUI")) {
				XmlTree ShowUI = settings.getChild("ShowUI");
				mShowUI = ShowUI.getAttributeValue<bool>("value");
			}
			if (settings.hasChild("FboWidth")) {
				XmlTree FboWidth = settings.getChild("FboWidth");
				mFboWidth = FboWidth.getAttributeValue<int>("value");
			}
			if (settings.hasChild("FboHeight")) {
				XmlTree FboHeight = settings.getChild("FboHeight");
				mFboHeight = FboHeight.getAttributeValue<int>("value");
			}
			if (settings.hasChild("OSCReceiverPort")) {
				XmlTree OSCReceiverPort = settings.getChild("OSCReceiverPort");
				mOSCReceiverPort = OSCReceiverPort.getAttributeValue<int>("value");
			}
			if (settings.hasChild("OSCDestinationPort")) {
				XmlTree OSCDestinationPort = settings.getChild("OSCDestinationPort");
				mOSCDestinationPort = OSCDestinationPort.getAttributeValue<int>("value");
			}
			if (settings.hasChild("OSCDestinationHost")) {
				XmlTree OSCDestinationHost = settings.getChild("OSCDestinationHost");
				mOSCDestinationHost = OSCDestinationHost.getAttributeValue<string>("value");
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
	mRenderWidth = 1024;
	mRenderHeight = 768;
	mRenderXY = mLeftRenderXY = mRightRenderXY = mPreviewRenderXY = vec2(0.0);
	mRenderPosXY = vec2(0.0, 320.0);
	mRenderResoXY = vec2(mRenderWidth, mRenderHeight);
	mRenderResolution = ivec2(mRenderWidth, mRenderHeight);
	mPreviewFragXY = vec2(0.0, 0.0);
	mWindowToCreate = 0;

	// spout
	mMemoryMode = false;
	mUseDX9 = false;

	mUIRefresh = 1;
	mOptimizeUI = false;
	mShowUI = true;
	mShowConsole = false;
	FPSColor = ColorA(0.0f, 1.0f, 0.0f, 1.0f);
	ColorGreen = ColorA(0.0f, 1.0f, 0.0f, 1.0f);
	ColorRed = ColorA(1.0f, 0.0f, 0.0f, 1.0f);

	// tempo
	mTempo = 166.0;
	mUseTimeWithTempo = false;
	iDeltaTime = 60 / mTempo;
	iTempoTime = 0.0;
	iTimeFactor = 1.0;
	//audio
	// audio in multiplication factor
	mAudioMultFactor = 1.0;
	mUseLineIn = true;
	maxVolume = 0.0f;
	mAudioTextureIndex = 0;
	mData = new float[1024];
	for (int i = 0; i < 1024; i++)
	{
		mData[i] = 0;
	}
	for (int i = 0; i < 4; i++)
	{
		iFreqs[i] = i;
	}
	// shader uniforms
	iGlobalTime = 1.0f;
	iResolution = vec3(mRenderWidth, mRenderHeight, 1.0);
	for (int i = 0; i < 4; i++)
	{
		iChannelTime[i] = i;
	}
	for (int i = 0; i < 4; i++)
	{
		iChannelResolution[i] = vec3(mRenderWidth, mRenderHeight, 1.0);
	}
	iDebug = iFade = iLight = iLightAuto = iRepeat = false;
	iFps = 60.0;
	iShowFps = true;
	iMouse = vec4(mRenderWidth / 2, mRenderHeight / 2, 1.0, 1.0);
	iGreyScale = false;

	// transition
	iTransition = 0;
	iAnim = 0.0;
	mTransitionDuration = 1.0f;


	// fbo
	mFboWidth = 640;
	mFboHeight = 360;
	mPreviewWidth = 156;
	mPreviewHeight = 88;
	mCurrentShadaFboIndex = 0;
	mMixFboIndex = 1;
	mLeftFboIndex = 2;
	mRightFboIndex = 3;

	mWarpCount = 3;
	// OSC
	mOSCDestinationHost = "127.0.0.1";
	mOSCDestinationPort = 9009;
	mOSCReceiverPort = 5005;
	OSCMsg = "OSC listening on port 5005";
	// colors
	mLockFR = mLockFG = mLockFB = mLockFA = mLockBR = mLockBG = mLockBB = mLockBA = false;
	tFR = tFG = tFB = tFA = tBR = tBG = tBB = tBA = false;

	for (int a = 0; a < 8; a++)
	{
		iChannels[a] = a;
		iWarpFboChannels[a] = a;
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
