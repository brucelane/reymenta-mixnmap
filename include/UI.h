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
// panels
#include "SlidersPanel.h"
#include "WarpPanel.h"

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
		UI( ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, ci::app::WindowRef aWindow );
		static UIRef						create( ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, ci::app::WindowRef aWindow );

		void								setup();
		void								draw();
		void								update();
		void								resize();

		void								mouseDown(MouseEvent event);
		void								mouseUp(MouseEvent event);
		void								mouseDrag(MouseEvent event);
		void								keyDown(KeyEvent event);

		void								fullscreen(const bool &pressed);
		void								toggleGlitch(const bool &pressed);
		void								toggleOptimizeUI(const bool &pressed);
		void								toggleAudioReactive(const bool &pressed);
		void								toggleVignette(const bool &pressed);
		void								toggleInvert(const bool &pressed);
		void								toggleLight(const bool &pressed);
		void								toggleLightAuto(const bool &pressed);
		void								toggleFade(const bool &pressed);
		void								toggleGreyScale(const bool &pressed);
		void								InstantBlack(const bool &pressed);
		void								debug(const bool &pressed);
		void								showFps(const bool &pressed);
		void								toggleUseTimeWithTempo(const bool &pressed);
		// panels
		void								useLineIn(const bool &pressed);
		void								toggleWarpPanel(const bool &pressed) { mWarpPanel->toggleVisibility(); };
		void								setTimeFactor(const int &aTimeFactor, const bool &pressed);
		void								setUIRefresh(const int &aFrames, const bool &pressed);
		void								setTextureIndex(const int &aTextureIndex, const bool &pressed);
		void								setShadaIndex(const int &aShadaIndex, const bool &pressed);
		void								createRenderWindow(const int &aIndex, const bool &pressed) { mParameterBag->mWindowToCreate = aIndex; }
		void								deleteRenderWindows(const bool &pressed) { mParameterBag->mWindowToCreate = 2; };

		void								lockFR(const bool &pressed) { mParameterBag->mLockFR = pressed; };
		void								lockFG(const bool &pressed) { mParameterBag->mLockFG = pressed; };
		void								lockFB(const bool &pressed) { mParameterBag->mLockFB = pressed; };
		void								lockFA(const bool &pressed) { mParameterBag->mLockFA = pressed; };
		void								lockBR(const bool &pressed) { mParameterBag->mLockBR = pressed; };
		void								lockBG(const bool &pressed) { mParameterBag->mLockBG = pressed; };
		void								lockBB(const bool &pressed) { mParameterBag->mLockBB = pressed; };
		void								lockBA(const bool &pressed) { mParameterBag->mLockBA = pressed; };
		void								tempoFR(const bool &pressed);
		void								resetFR(const bool &pressed);
		void								tempoFG(const bool &pressed);
		void								resetFG(const bool &pressed);
		void								tempoFB(const bool &pressed);
		void								resetFB(const bool &pressed);
		void								tempoFA(const bool &pressed);
		void								resetFA(const bool &pressed);
		void								tempoBR(const bool &pressed);
		void								resetBR(const bool &pressed);
		void								tempoBG(const bool &pressed);
		void								resetBG(const bool &pressed);
		void								tempoBB(const bool &pressed);
		void								resetBB(const bool &pressed);
		void								tempoBA(const bool &pressed);
		void								resetBA(const bool &pressed);
		// tempo
		void								tapTempo( const bool &pressed );
		void								calculateTempo();

		MinimalUI::UIElementRef				sliderRed, sliderGreen, sliderBlue, sliderAlpha;
		MinimalUI::UIElementRef				sliderBackgroundRed, sliderBackgroundGreen, sliderBackgroundBlue, sliderBackgroundAlpha;
		MinimalUI::UIElementRef				labelXY, labelPosXY;
		MinimalUI::UIElementRef				sliderRenderXY, sliderRenderPosXY, sliderXSpeed, sliderYSpeed, sliderPreviewTextureXY, sliderPreviewShadaXY;

		MinimalUI::UIElementRef				labelOSC, labelError, labelLayer;
		MinimalUI::UIElementRef				buttonChannels;
		vector<MinimalUI::UIElementRef>		buttonShada, buttonTexture, labelTexture, labelShada;
		
		MinimalUI::UIElementRef				fpsMvg, volMvg, tempoMvg;
		MinimalUI::UIElementRef				sliderAudioMul;
		void								toggleVisibility() { mVisible ? hide() : show(); }
		void								show();
		void								hide();
		void								shutdown();
		//LibraryPanelRef						mLibraryPanel;
		WarpPanelRef						mWarpPanel;
		void								addShadaControls();
		void								addTextureControls();
		void								addMixControls();

		int									getTextureButtonsCount() { return buttonTexture.size(); };
		void								createWarp();

	private:
		void								setupGlobal();
		void								setupMiniControl();
		void								setupSliders();
		void								setupTextures();
		void								setupShaders();
		void								setupLibrary();

		// windows mgmt
		MinimalUI::UIControllerRef			mMiniControl, gParams, tParams, sParams, mixParams;
		// panels
		SlidersPanelRef						mSlidersPanel;
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
		// Spaghetti
		SpaghettiRef						mSpaghetti;

		Anim<float>							mTimer;

		// windows to create, should be the same as in ParameterBag
		static const int					MIDI_IN = 6;
		static const int					SPOUT = 7;

		// math
		string								formatNumber(float f);
		// tempo
		std::deque <double>					buffer;
		ci::Timer							timer;
		int									counter;
		double								averageTime;
		double								currentTime;
		double								startTime;
		float								previousTime;
		int									beatIndex; //0 to 3
		// settings
		void								saveSettings(const bool &pressed = true);
		void								restoreSettings(const bool &pressed = true);
		void								resetSettings(const bool &pressed = true);

		// library panel
		void								setLeftInput(const int &aIndex, const bool &pressed);
		void								setRightInput(const int &aIndex, const bool &pressed);
		void								setPreview(const int &aIndex, const bool &pressed);

		vector<MinimalUI::UIElementRef>		buttonSelect, buttonLeft, buttonRight, sliderCrossfade;
		MinimalUI::UIElementRef				sliderLeftRenderXY, sliderRightRenderXY, sliderMixRenderXY;
		vector<float>						iCrossfade;  // from 0 left to 1 right
};
}