#include "SlidersPanel.h"

using namespace Reymenta;
/*
4:3 w h
btn: 48 36
tex: 76 57
pvw: 156 88
*/
SlidersPanel::SlidersPanel(ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, WindowRef aWindow)
{
	mParameterBag = aParameterBag;
	mShaders = aShadersRef;
	mTextures = aTexturesRef;
	mWindow = aWindow;

	// zoom
	defaultZoom = 1.0f;
	minZoom = 0.1;
	maxZoom = 5.0;
	tZoom = autoZoom = false;
	// exposure
	defaultExposure = 1.0;
	minExposure = 0.0001;
	maxExposure = 2.0;
	tExposure = autoExposure = false;
	// ratio
	defaultRatio = 20.0;
	minRatio = 255.0;
	maxRatio = 0.00000000001;
	tRatio = autoRatio = false;
	// RotationSpeed
	defaultRotationSpeed = 1.0;
	minRotationSpeed = -10.0;
	maxRotationSpeed = 10.0;
	tRotationSpeed = autoRotationSpeed = false;

	setupParams();
}

SlidersPanelRef SlidersPanel::create(ParameterBagRef aParameterBag, ShadersRef aShadersRef, TexturesRef aTexturesRef, WindowRef aWindow)
{
	return shared_ptr<SlidersPanel>(new SlidersPanel(aParameterBag, aShadersRef, aTexturesRef, aWindow));
}

void SlidersPanel::setupParams()
{
	mParams = UIController::create("{ \"visible\":true, \"x\":1074, \"y\":150, \"height\":440, \"depth\":250, \"panelColor\":\"0x44282828\" }");
	mParams->DEFAULT_UPDATE_FREQUENCY = 12;
	// set custom fonts for a UIController
	mParams->setFont("label", mParameterBag->mLabelFont);
	mParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	mParams->setFont("icon", mParameterBag->mIconFont);
	mParams->setFont("header", mParameterBag->mHeaderFont);
	mParams->setFont("body", mParameterBag->mBodyFont);
	mParams->setFont("footer", mParameterBag->mFooterFont);

	// ratio
	sliderRatio = mParams->addToggleSlider("ratio", &mParameterBag->controlValues[11], "a", std::bind(&SlidersPanel::lockRatio, this, std::placeholders::_1), "{ \"clear\":false, \"width\":" + toString(mParameterBag->mPreviewWidth - 39) + ", \"min\":" + toString(minRatio) + ", \"max\":" + toString(maxRatio) + " }", "{ \"width\":9, \"stateless\":false, \"group\":\"ratio\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("t", std::bind(&SlidersPanel::tempoRatio, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"ratio\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("x", std::bind(&SlidersPanel::resetRatio, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"ratio\", \"exclusive\":true, \"pressed\":true, \"clear\":false }");
	mParams->addSlider("m", &minRatio, "{ \"min\":" + toString(minRatio) + ", \"max\":" + toString(maxRatio) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true, \"clear\":false }");
	mParams->addSlider("M", &maxRatio, "{ \"min\":" + toString(minRatio) + ", \"max\":" + toString(maxRatio) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true }");

	// exposure
	sliderExposure = mParams->addToggleSlider("exposure", &mParameterBag->controlValues[14], "a", std::bind(&SlidersPanel::lockExposure, this, std::placeholders::_1), "{ \"clear\":false, \"width\":" + toString(mParameterBag->mPreviewWidth - 39) + ", \"min\":" + toString(minExposure) + ", \"max\":" + toString(maxExposure) + " }", "{ \"width\":9, \"stateless\":false, \"group\":\"exposure\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("t", std::bind(&SlidersPanel::tempoExposure, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"exposure\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("x", std::bind(&SlidersPanel::resetExposure, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"exposure\", \"exclusive\":true, \"pressed\":true, \"clear\":false }");
	mParams->addSlider("m", &minExposure, "{ \"min\":" + toString(minExposure) + ", \"max\":" + toString(maxExposure) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true, \"clear\":false }");
	mParams->addSlider("M", &maxExposure, "{ \"min\":" + toString(minExposure) + ", \"max\":" + toString(maxExposure) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true }");
	// zoom
	//sliderZoom = mParams->addSlider("zoom", &mParameterBag->iZoom, "{ \"min\":5.0, \"max\":0.1 }");
	sliderZoom = mParams->addToggleSlider("zoom", &mParameterBag->controlValues[13], "a", std::bind(&SlidersPanel::lockZoom, this, std::placeholders::_1), "{ \"clear\":false, \"width\":" + toString(mParameterBag->mPreviewWidth - 39) + ", \"min\":" + toString(minZoom) + ", \"max\":" + toString(maxZoom) + " }", "{ \"width\":9, \"stateless\":false, \"group\":\"zoom\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("t", std::bind(&SlidersPanel::tempoZoom, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"zoom\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("x", std::bind(&SlidersPanel::resetZoom, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"zoom\", \"exclusive\":true, \"pressed\":true, \"clear\":false }");
	mParams->addSlider("m", &minZoom, "{ \"min\":" + toString(minZoom) + ", \"max\":" + toString(maxZoom) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true, \"clear\":false }");
	mParams->addSlider("M", &maxZoom, "{ \"min\":" + toString(minZoom) + ", \"max\":" + toString(maxZoom) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true }");

	sliderSpeed = mParams->addSlider("speed", &mParameterBag->controlValues[12], "{ \"min\":1.0, \"max\":255.0, \"nameColor\":\"0xFFFFFFFF\" }");
	// rotation speed
	sliderRotationSpeed = mParams->addToggleSlider("rotation", &mParameterBag->controlValues[19], "a", std::bind(&SlidersPanel::lockRotationSpeed, this, std::placeholders::_1), "{ \"width\":" + toString(mParameterBag->mPreviewWidth - 39) + ", \"clear\":false, \"min\":" + toString(minRotationSpeed) + ", \"max\":" + toString(maxRotationSpeed) + " }", "{ \"width\":9, \"stateless\":false, \"group\":\"rotationspeed\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("t", std::bind(&SlidersPanel::tempoRotationSpeed, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"rotationspeed\", \"exclusive\":true, \"clear\":false }");
	mParams->addButton("x", std::bind(&SlidersPanel::resetRotationSpeed, this, std::placeholders::_1), "{ \"width\":9, \"stateless\":false, \"group\":\"rotationspeed\", \"exclusive\":true, \"pressed\":true, \"clear\":false }");
	mParams->addSlider("m", &minRotationSpeed, "{ \"min\":" + toString(minRotationSpeed) + ", \"max\":" + toString(maxRotationSpeed) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true, \"clear\":false }");
	mParams->addSlider("M", &maxRotationSpeed, "{ \"min\":" + toString(minRotationSpeed) + ", \"max\":" + toString(maxRotationSpeed) + ", \"handleVisible\":false, \"width\":16, \"vertical\":true }");

	sliderBlendmode = mParams->addSlider("blendmode", &mParameterBag->controlValues[15], "{ \"min\":0.0, \"max\":27.0 }");
	sliderSteps = mParams->addSlider("steps", &mParameterBag->controlValues[16], "{ \"min\":1.0, \"max\":128.0, \"nameColor\":\"0xFFFFFFFF\" }");
	sliderPixelate = mParams->addSlider("pixelate", &mParameterBag->controlValues[18], "{ \"min\":0.01, \"max\":1.0 }");
	sliderPreviewCrossfade = mParams->addSlider("PreviewXFade", &mParameterBag->iPreviewCrossfade, "{ \"min\":0.0, \"max\":1.0 }");
	sliderCrossfade = mParams->addSlider("xFade", &mParameterBag->iCrossfade, "{ \"min\":0.0, \"max\":1.0 }");


}


void SlidersPanel::update()
{
	if (mVisible && !mParameterBag->mOptimizeUI)
	{
		sliderRatio->setName("11 ratio:" + toString(floor(mParameterBag->controlValues[11])) + "\n" + toString(floor(minRatio)) + "-" + toString(floor(maxRatio)));
		sliderSpeed->setName("12 speed:" + toString(floor(mParameterBag->controlValues[12])));
		sliderZoom->setName("13 zoom:" + toString(floor(mParameterBag->controlValues[13])) + "\n" + toString(floor(minZoom)) + "-" + toString(floor(maxZoom)));
		sliderExposure->setName("14 exposure:" + toString(floor(mParameterBag->controlValues[14])) + "\n" + toString(floor(minExposure)) + "-" + toString(floor(maxExposure)));
		sliderBlendmode->setName("15 blendmode:" + toString(floor(mParameterBag->controlValues[15])));
		sliderSteps->setName("16 steps:" + toString(floor(mParameterBag->controlValues[16])));
		// crossfade
		sliderPreviewCrossfade->setBackgroundColor(Color::gray(mParameterBag->iPreviewCrossfade/2.0));
		sliderPreviewCrossfade->setName("Pvw xFade:" + formatNumber(mParameterBag->iPreviewCrossfade));
		sliderCrossfade->setName("17 xFade:" + formatNumber(mParameterBag->iCrossfade));
		sliderPixelate->setName("18 pixelate:" + toString(floor(mParameterBag->controlValues[18])));
		sliderRotationSpeed->setName("19 rotation speed:" + toString(floor(mParameterBag->controlValues[19])));
		mParams->update();
	}
}
void SlidersPanel::draw()
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
string SlidersPanel::formatNumber(float f)
{
	f *= 100;
	f = (float)((int)f);
	return toString(f);
}
void SlidersPanel::resize()
{
	mParams->resize();
}

void SlidersPanel::show()
{
	mVisible = true;
}

void SlidersPanel::hide()
{
	mVisible = false;
}

void SlidersPanel::tempoZoom(const bool &pressed)
{
	tZoom = pressed;
	if (!pressed) resetZoom(pressed);
}
void SlidersPanel::resetZoom(const bool &pressed)
{
	autoZoom = false;
	tZoom = false;
	mParameterBag->controlValues[13] = defaultZoom;
}

void SlidersPanel::tempoRotationSpeed(const bool &pressed)
{
	tRotationSpeed = pressed;
	if (!pressed) resetRotationSpeed(pressed);
}
void SlidersPanel::resetRotationSpeed(const bool &pressed)
{
	autoRotationSpeed = false;
	tRotationSpeed = false;
	mParameterBag->controlValues[19] = defaultRotationSpeed;
}

void SlidersPanel::tempoExposure(const bool &pressed)
{
	tExposure = pressed;
	if (!pressed) resetExposure(pressed);
}
void SlidersPanel::resetExposure(const bool &pressed)
{
	autoExposure = false;
	tExposure = false;
	mParameterBag->controlValues[14] = defaultExposure;
}
void SlidersPanel::tempoRatio(const bool &pressed)
{
	tRatio = pressed;
	if (!pressed) resetRatio(pressed);
}
void SlidersPanel::resetRatio(const bool &pressed)
{
	autoRatio = false;
	tRatio = false;
	mParameterBag->controlValues[11] = defaultRatio;
}