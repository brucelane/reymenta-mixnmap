#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Timeline.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/TextureFont.h"

#include "Resources.h"
// parameters
#include "ParameterBag.h"
#include "UIController.h"
#include "UIElement.h"
// textures
#include "Textures.h"
// shaders
#include "Shaders.h"

#if defined( CINDER_COCOA )
#include "cinderSyphon.h"
#else
#include "Spout.h"
#endif

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace MinimalUI;

namespace Reymenta
{

	typedef std::shared_ptr<class OutputPanel> OutputPanelRef;

	class OutputPanel
	{

	public:
		OutputPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);
		static OutputPanelRef create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);

		void setup();
		void update();
		void draw();
		void resize();
		void shutdown();
		bool isSetup() { return mSetup; }

		void toggleVisibility() { mVisible ? hide() : show(); }

		void show();
		void hide();

		MinimalUI::UIElementRef				labelOutput;
		MinimalUI::UIElementRef				labelOutputResoXY, sliderOutputResoXY;
	private:
		void toggleSendToOutput(const bool &pressed);
		void toggleDirectRender(const bool &pressed);
		void createRenderWindow(const int &aIndex, const bool &pressed) { mParameterBag->mWindowToCreate = aIndex; }
		void deleteRenderWindows(const bool &pressed) { mParameterBag->mWindowToCreate = 2; };
		void setupParams();

		void releaseGroup(const std::string &aGroup);
		MinimalUI::UIControllerRef mParams;
		ci::app::WindowRef mWindow;
		ci::signals::scoped_connection mCbMouseDown, mCbKeyDown;
		bool mVisible;

		bool mSetup;
		// Parameters
		ParameterBagRef				mParameterBag;
		// Shaders
		ShadersRef					mShaders;
		// Textures
		TexturesRef					mTextures;
		//fbo
		gl::Fbo mSpoutFbo;
		gl::GlslProg aShader;
		
		// spout
		SpoutSender spoutsender;                    // Create a Spout sender object
		bool bInitialized;                          // true if a sender initializes OK
		bool bMemoryMode;                           // tells us if texture share compatible
		unsigned int g_Width, g_Height;             // size of the texture being sent out
		char SenderName[256];                       // sender name 
		gl::Texture spoutTexture;                   // Local Cinder texture used for sharing
	};
}