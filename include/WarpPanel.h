#pragma once

// parameters
#include "ParameterBag.h"
// shaders
#include "Shaders.h"
// textures
#include "Textures.h"

#include "UIElement.h"
#include "UIController.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace MinimalUI;

namespace Reymenta
{
	typedef std::shared_ptr<class WarpPanel> WarpPanelRef;

	class WarpPanel {
	public:
		WarpPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef);
		static WarpPanelRef create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef);
		void toggleVisibility() { mVisible ? hide() : show(); }
		void update();
		void draw();
		void resize();
		void show();
		void hide();
	private:

		ParameterBagRef						mParameterBag;
		ShadersRef							mShaders;
		TexturesRef							mTextures;
		bool								mVisible;
		int									warpIndex;
		void								setupParams();
		void								setCurrentIndex(const int &aIndex, const bool &pressed);
		void								setCurrentFboIndex(const int &aIndex, const bool &pressed);
		MinimalUI::UIElementRef				buttonIndex[8], labelFboIndex[8];
		MinimalUI::UIElementRef				currentIndexLabel;
		MinimalUI::UIControllerRef			mParams;
	};
}