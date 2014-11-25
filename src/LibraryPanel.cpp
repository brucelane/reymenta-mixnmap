#include "LibraryPanel.h"

using namespace Reymenta;
/*
4:3 w h
btn: 48 36
tex: 76 57
pvw: 156 88
*/
LibraryPanel::LibraryPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef)
{
	mParameterBag = aParameterBag;
	mShaders = aShadersRef;
	mTextures = aTexturesRef;

	setupParams();
}

LibraryPanelRef LibraryPanel::create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef)
{
	return shared_ptr<LibraryPanel>(new LibraryPanel(aParameterBag, aTexturesRef, aShadersRef));
}

void LibraryPanel::setupParams()
{
	mParams = UIController::create("{ \"visible\":true, \"x\":356, \"y\":150, \"width\":500, \"height\":530, \"depth\":203, \"panelColor\":\"0x44482828\" }");
	//mParams->DEFAULT_UPDATE_FREQUENCY = 12;
	// set custom fonts for a UIController
	mParams->setFont("label", mParameterBag->mLabelFont);
	mParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	mParams->setFont("icon", mParameterBag->mIconFont);
	mParams->setFont("header", mParameterBag->mHeaderFont);
	mParams->setFont("body", mParameterBag->mBodyFont);
	mParams->setFont("footer", mParameterBag->mFooterFont);

	mParams->addLabel("Texture mixing", "{ \"clear\":false, \"width\":64 }");
	flipButton = mParams->addButton("Flip", std::bind(&LibraryPanel::flipLibraryCurrentFbo, this, std::placeholders::_1), "{ \"width\":48, \"stateless\":false, \"pressed\":true }");
	sliderLeftRenderXY = mParams->addSlider2D("LeftXY", &mParameterBag->mLeftRenderXY, "{ \"clear\":false, \"minX\":-2.0, \"maxX\":2.0, \"minY\":-2.0, \"maxY\":2.0, \"width\":" + toString(mParameterBag->mPreviewWidth) + " }");
	sliderRightRenderXY = mParams->addSlider2D("RightXY", &mParameterBag->mRightRenderXY, "{ \"clear\":false, \"minX\":-2.0, \"maxX\":2.0, \"minY\":-2.0, \"maxY\":2.0, \"width\":" + toString(mParameterBag->mPreviewWidth) + " }");
	sliderMixRenderXY = mParams->addSlider2D("MixXY", &mParameterBag->mPreviewRenderXY, "{ \"minX\":-2.0, \"maxX\":2.0, \"minY\":-2.0, \"maxY\":2.0, \"width\":" + toString(mParameterBag->mPreviewWidth) + " }");
	mParams->addSlider("LZoom", &mParameterBag->iZoomLeft, "{ \"clear\":false, \"width\":" + toString(mParameterBag->mPreviewWidth) + ", \"min\":0.1, \"max\":5.0 }");
	mParams->addSlider("RZoom", &mParameterBag->iZoomRight, "{ \"width\":" + toString(mParameterBag->mPreviewWidth) + ", \"min\":0.1, \"max\":5.0 }");
}
void LibraryPanel::addButtons()
{
	int i = buttonLeft.size();
	//labelInput.push_back(mParams->addLabel("In", "{ \"clear\":false, \"width\":18}"));
	buttonLeft.push_back(mParams->addButton("L", std::bind(&LibraryPanel::setLeftInput, this, i, std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":true, \"group\":\"c0\", \"exclusive\":true }"));
	buttonRight.push_back(mParams->addButton("R", std::bind(&LibraryPanel::setRightInput, this, i, std::placeholders::_1), "{  \"clear\":false, \"width\":48, \"stateless\":true, \"group\":\"c1\", \"exclusive\":true }"));
	buttonSelect.push_back(mParams->addButton(toString(i), std::bind(&LibraryPanel::setPreview, this, i, std::placeholders::_1), "{ \"clear\":false, \"width\":48, \"stateless\":false, \"group\":\"pvw\", \"exclusive\":true }"));
	sliderCrossfade.push_back(mParams->addSlider("xFade", &mTextures->iCrossfade[i], "{ \"min\":0.0, \"max\":1.0, \"width\":96 }"));
}

void LibraryPanel::flipLibraryCurrentFbo(const bool &pressed)
{
	mTextures->flipMixFbo(pressed);
}
void LibraryPanel::setPreview(const int &aIndex, const bool &pressed)
{

}
void LibraryPanel::setCurrentFbo(const int &aIndex, const bool &pressed)
{
}
void LibraryPanel::setLeftInput(const int &aIndex, const bool &pressed)
{
	WarpInput wi = mTextures->setInput(aIndex, true);
	buttonLeft[aIndex]->setName(toString( wi.leftIndex));
	if (wi.leftMode = 0)
	{
		buttonLeft[aIndex]->setBackgroundTexture(mTextures->getTexture(wi.leftIndex));
	}
	else
	{
		buttonLeft[aIndex]->setBackgroundTexture(mTextures->getFboTexture(wi.leftIndex));
	}
}
void LibraryPanel::setRightInput(const int &aIndex, const bool &pressed)
{
	WarpInput wi = mTextures->setInput(aIndex, false);
	buttonRight[aIndex]->setName(toString(wi.rightIndex));
	if (wi.rightMode = 0)
	{
		buttonRight[aIndex]->setBackgroundTexture(mTextures->getTexture(wi.rightIndex));
	}
	else
	{
		buttonRight[aIndex]->setBackgroundTexture(mTextures->getFboTexture(wi.rightIndex));
	}

}
void LibraryPanel::update()
{
	// animation	
	if (mVisible)
	{
		mParams->update();

		if (!mParameterBag->mOptimizeUI)
		{
			if (getElapsedFrames() % (mParameterBag->mUIRefresh * mParameterBag->mUIRefresh * mParameterBag->mUIRefresh) == 0)
			{
				//TODO buttonInput->setBackgroundTexture(mTextures->getShaderTexture());
				sliderLeftRenderXY->setBackgroundTexture(mTextures->getMixTexture(0));
				sliderRightRenderXY->setBackgroundTexture(mTextures->getMixTexture(1));
				sliderMixRenderXY->setBackgroundTexture(mTextures->getMixTexture(2));

			}
		}
	}
}
void LibraryPanel::draw()
{
	if (mVisible)
	{
		// normal alpha blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		mParams->draw();

		// needed because of what the ping pong fbo is doing, at least
		gl::disableAlphaBlending();
	}
}

void LibraryPanel::resize()
{
	mParams->resize();
}

void LibraryPanel::show()
{
	mVisible = true;
}

void LibraryPanel::hide()
{
	mVisible = false;
}
