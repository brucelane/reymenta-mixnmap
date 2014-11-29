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
		LibraryPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);
		static LibraryPanelRef create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);

		void draw();
		void update();
		void resize();

		void toggleVisibility() { mVisible ? hide() : show(); }
		void setCurrentFbo(const int &aIndex, const bool &pressed);

		void show();
		void hide();

		void setUpdateFrequency() { mParams->DEFAULT_UPDATE_FREQUENCY = 4 * mParameterBag->mUIRefresh; };
		void addButtons();
	private:
		void setupParams();
		void flipLibraryCurrentFbo(const bool &pressed);

		void setLeftInput(const int &aIndex, const bool &pressed);
		void setRightInput(const int &aIndex, const bool &pressed);
		void setPreview(const int &aIndex, const bool &pressed);

		vector<MinimalUI::UIElementRef>	buttonSelect, buttonLeft, buttonRight, sliderCrossfade;
		
		MinimalUI::UIElementRef			flipButton, sliderLeftRenderXY, sliderRightRenderXY, sliderMixRenderXY;
		MinimalUI::UIControllerRef		mParams;
		bool							mVisible;

		ParameterBagRef					mParameterBag;
		TexturesRef						mTextures;
	};
}