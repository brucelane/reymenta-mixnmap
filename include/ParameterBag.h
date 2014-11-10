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

		// params
		std::string					mOSCDestinationHost;
		int							mOSCDestinationPort;
		int							mOSCReceiverPort;
		std::string					OSCMsg;

		// render windows
		int							mRenderWidth;
		int							mRenderHeight;
		vec2						mRenderXY, mLeftRenderXY, mRightRenderXY, mPreviewRenderXY;
		vec2						mRenderPosXY;
		vec2						mRenderResoXY;
		ivec2						mRenderResolution;        // render resolution (replaces iResolution which increments magically)

		// windows and params
		bool						mShowUI;
		bool						mShowConsole;
		int							mMainDisplayWidth;
		int							mRenderX;
		int							mRenderY;
		int							mDisplayCount;

		// Textures
		bool						mOriginUpperLeft;
		int							currentSelectedIndex;

		// OSC/MIDI/JSON controlled UI and params
		map<int, float>				controlValues;
		// indexes for textures
		map<int, int>				iChannels;
		// fbos
		int							mFboWidth, mFboHeight;
		bool						mFlipFbo;
		// tap tempo
		float						mTempo;
		float						iDeltaTime;
		float						iTempoTime;
		float						iTimeFactor;
		bool						mUseTimeWithTempo;
		// shader uniforms
		float						iGlobalTime;        // shader playback time (in seconds)
		float						iChannelTime[4];
		vec3						iResolution;        // viewport resolution (in pixels)
		vec3						iChannelResolution[4];	// channel resolution (in pixels)
		vec4						iMouse;             // mouse pixel coords. xy: current (if MLB down), zw: click
		bool						iFade;
		bool						iRepeat;
		bool						iLight;
		bool						iLightAuto;
		float						iCrossfade, iPreviewCrossfade;
		bool						iShowFps;
		bool						iDebug;
		float						iFps;
		bool						iGreyScale;
		// transition
		int							iTransition;
		Anim<float>					iAnim;
		float						mTransitionDuration;
		// spout
		bool						mMemoryMode;			// tells us if texture share compatible
		bool						mUseDX9;				// use DirectX 9 mode
	private:
		const string settingsFileName = "MixnMapSettings.xml";
	};

}
