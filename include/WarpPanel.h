#pragma once

// parameters
#include "ParameterBag.h"
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
		WarpPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);
		static WarpPanelRef					create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);
		void								toggleVisibility() { mVisible ? hide() : show(); }
		void								update();
		void								draw();
		void								resize();
		void								show();
		void								hide();
		void								addButtons();
		int									getWarpsSize() { return buttonIndex.size(); };
	private:

		ParameterBagRef						mParameterBag;
		TexturesRef							mTextures;
		bool								mVisible;
		int									warpIndex;
		void								setupParams();
		void								setCurrentIndex(const int &aIndex, const bool &pressed);
		void								setCurrentFboIndex(const int &aIndex, const bool &pressed);
		vector<MinimalUI::UIElementRef>		buttonIndex, labelFboIndex;	
		MinimalUI::UIElementRef				currentIndexLabel;
		MinimalUI::UIControllerRef			mParams;
	};
}