#pragma once

#include "cinder/app/AppNative.h"
#include "cinder/Timeline.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

// Parameters
#include "ParameterBag.h"
#include "UIController.h"
#include "UIElement.h"
// shaders
#include "Shaders.h"
// textures
#include "Textures.h"
// Spaghetti
#include "Spaghetti.h"
// OSC
#include "OSCWrapper.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace MinimalUI;

namespace Reymenta 
{

	typedef std::shared_ptr<class UI> UIRef;

	class UI 
	{

	public:
		UI( ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, ci::app::WindowRef aWindow, OSCRef aOscRef );
		static UIRef						create(ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, ci::app::WindowRef aWindow, OSCRef aOscRef);

		void								setup();
		void								draw();
		void								update();
		void								resize();

		void								mouseDown(MouseEvent event);
		void								mouseUp(MouseEvent event);
		void								mouseDrag(MouseEvent event);
		void								keyDown(KeyEvent event);

		void								fullscreen(const bool &pressed);
		void								showFps(const bool &pressed);
		void								toggleUseTimeWithTempo(const bool &pressed);
		// panels
		void								setTextureIndex(const int &aTextureIndex, const bool &pressed);
		void								setShadaIndex(const int &aShadaIndex, const bool &pressed);
		void								createRenderWindow(const int &aIndex, const bool &pressed) { mParameterBag->mWindowToCreate = aIndex; }
		void								deleteRenderWindows(const bool &pressed) { mParameterBag->mWindowToCreate = 2; };
		void								setupOSCReceiver(const bool &pressed) { mOSC->setupReceiver(); };

		MinimalUI::UIElementRef				labelOSC, labelError, labelLayer;
		MinimalUI::UIElementRef				buttonChannels;
		vector<MinimalUI::UIElementRef>		buttonShada, buttonTexture, labelTexture, labelShada;
		
		MinimalUI::UIElementRef				fpsMvg;
		void								toggleVisibility() { mVisible ? hide() : show(); }
		void								show();
		void								hide();
		void								shutdown();
		void								addShadaControls();
		void								addTextureControls();
		void								addMixControls();
		void								addButtons();

		int									getTextureButtonsCount() { return buttonTexture.size(); };
		// warps
		void								createWarp();
		void								setCurrentIndex(const int &aIndex, const bool &pressed);
		int									getWarpsSize() { return buttonIndex.size(); };

	private:
		void								setupMiniControl();
		void								setupTextures();
		void								setupShaders();
		void								setupLibrary();
		void								setupWarps();

		// windows mgmt
		MinimalUI::UIControllerRef			mMiniControl, tParams, sParams, mixParams, wParams;
		// panels
		vector<MinimalUI::UIControllerRef>	mPanels;

		ci::app::WindowRef					mWindow;
		ci::signals::scoped_connection		mCbMouseDown, mCbKeyDown;
		int									mPrevState;
		bool								mVisible;
		bool								mSetupComplete;

		// Parameters
		ParameterBagRef						mParameterBag;
		// Shaders
		ShadersRef							mShaders;
		// Textures
		TexturesRef							mTextures;
		// osc
		OSCRef								mOSC;

		// Spaghetti
		SpaghettiRef						mSpaghetti;

		// math
		string								formatNumber(float f);

		// settings
		void								saveSettings(const bool &pressed = true);
		void								restoreSettings(const bool &pressed = true);
		void								resetSettings(const bool &pressed = true);

		// library panel
		void								setLeftInput(const int &aIndex, const bool &pressed);
		void								setRightInput(const int &aIndex, const bool &pressed);
		void								setPreview(const int &aIndex, const bool &pressed);
		// warps
		void								setCurrentFboIndex(const int &aIndex, const bool &pressed);
		int									warpIndex;
		vector<MinimalUI::UIElementRef>		buttonIndex, labelFboIndex;
		MinimalUI::UIElementRef				currentIndexLabel;

		vector<MinimalUI::UIElementRef>		buttonSelect, buttonLeft, buttonRight, sliderCrossfade;
		vector<float>						iCrossfade;  // from 0 left to 1 right
};
}