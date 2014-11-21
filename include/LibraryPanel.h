#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Timeline.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/TextureFont.h"

#include "Resources.h"
#include "ParameterBag.h"
#include "UIController.h"
#include "UIElement.h"
// shaders
#include "Shaders.h"
// textures
#include "Textures.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace MinimalUI;

namespace Reymenta
{

	typedef std::shared_ptr<class LibraryPanel> LibraryPanelRef;

	class LibraryPanel
	{

	public:
		LibraryPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef);
		static LibraryPanelRef create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef);

		void draw();
		void update();
		void resize();

		void toggleVisibility() { mVisible ? hide() : show(); }
		void setCurrentFbo(const int &aIndex, const bool &pressed);
		void setInput(const int &aIndex, const bool &pressed);

		void show();
		void hide();

		void setUpdateFrequency() { mParams->DEFAULT_UPDATE_FREQUENCY = 4 * mParameterBag->mUIRefresh; };
	private:
		void setupParams();
		void flipLibraryCurrentFbo(const bool &pressed);

		void setLeftFragActive(const int &aIndex, const bool &pressed);
		void setRightFragActive(const int &aIndex, const bool &pressed);

		vector<MinimalUI::UIElementRef>	labelInput, buttonFrag;
		
		MinimalUI::UIElementRef			flipButton, sliderLeftRenderXY, sliderRightRenderXY, sliderMixRenderXY;
		MinimalUI::UIControllerRef		mParams;
		bool							mVisible;

		ParameterBagRef					mParameterBag;
		ShadersRef						mShaders;
		TexturesRef						mTextures;
	};
}