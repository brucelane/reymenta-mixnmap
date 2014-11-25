/**
* \file ParameterBag.h
* \author Bruce LANE/Nathan Selikoff
* \date 20 november 2014
*
* Parameters for all classes.
*
*/
#pragma once

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Json.h"
#include "cinder/Xml.h"

// webcam
#include "cinder/Capture.h"
#include "cinder/Surface.h"
#include "cinder/MayaCamUI.h"
// fonts
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta {

	typedef std::shared_ptr<class ParameterBag> ParameterBagRef;

	class ParameterBag
	{
	public:
		ParameterBag();
		static ParameterBagRef create();

		bool save();
		bool restore();
		void reset();
		// fonts
		ci::Font mLabelFont, mSmallLabelFont, mIconFont, mHeaderFont, mBodyFont, mFooterFont;

		// params
		std::string					mOSCDestinationHost;
		int							mOSCDestinationPort;
		int							mOSCReceiverPort;
		std::string					OSCMsg;
		int							mPreviewWidth, mPreviewHeight;
		std::string					InfoMsg;
		// render windows
		int							mRenderWidth;
		int							mRenderHeight;
		vec2						mRenderXY, mLeftRenderXY, mRightRenderXY, mPreviewRenderXY;
		vec2						mRenderPosXY;
		vec2						mRenderResoXY;
		ivec2						mRenderResolution;        // render resolution (replaces iResolution which increments magically)
		vec2						mPreviewFragXY;

		// windows and params
		bool						mShowUI;
		bool						mShowConsole;
		int							mMainDisplayWidth;
		int							mMainDisplayHeight;
		int							mRenderX;
		int							mRenderY;
		int							mDisplayCount;
		int							mWindowToCreate;
		bool						mOptimizeUI;
		int							mUIRefresh;
		ColorA						FPSColor;
		ColorA						ColorGreen;
		ColorA						ColorRed;
		//! maximum bound for fbos, shaders, textures
		static const int			MAX = 8;
		// audio
		float						*mData;
		float						maxVolume;
		bool						mUseLineIn;
		float						mAudioMultFactor;
		float						iFreqs[4];
		int							mAudioTextureIndex;
		// Textures
		bool						mOriginUpperLeft;

		// OSC/MIDI/JSON controlled UI and params
		map<int, float>				controlValues;
		// indexes for textures
		map<int, int>				iChannels;
		// fbo indexes for warping
		map<int, int>				iWarpFboChannels;
		// fbos
		int							mFboWidth, mFboHeight;
		bool						mFlipFbo;
		int							mWarpCount;
		int							mCurrentShadaFboIndex;
		int							mMixFboIndex;
		int							mLeftFboIndex, mRightFboIndex;
		float						iZoomLeft, iZoomRight;
		// tap tempo
		float						mTempo;
		float						iDeltaTime;
		float						iTempoTime;
		float						iTimeFactor;
		bool						mUseTimeWithTempo;
		// shader uniforms
		//! shader playback time (in seconds)
		float						iGlobalTime;			
		float						iChannelTime[4];
		//! viewport resolution (in pixels)
		vec3						iResolution;
		//! channel resolution (in pixels)			
		vec3						iChannelResolution[4];
		//! mouse pixel coords. xy: current (if MLB down), zw: click	
		vec4						iMouse;					
		bool						iFade;
		bool						iRepeat;
		bool						iLight;
		bool						iLightAuto;
		float						iCrossfade, iPreviewCrossfade;
		bool						iShowFps;
		bool						iDebug;
		float						iFps;
		bool						iGreyScale;
		// colors
		bool						tFR, tFG, tFB, tFA, tBR, tBG, tBB, tBA;
		bool						mLockFR, mLockFG, mLockFB, mLockFA, mLockBR, mLockBG, mLockBB, mLockBA;
		// transition
		int							iTransition;
		Anim<float>					iAnim;
		float						mTransitionDuration;
		// spout
		bool						mMemoryMode;			// tells us if texture share compatible
		bool						mUseDX9;				// use DirectX 9 mode, should not be set to true, unless old beta of Spout
	private:
		const string settingsFileName = "MixnMapSettings.xml";
	};

}
