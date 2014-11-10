#include "OutputPanel.h"

using namespace Reymenta;
/*
4:3 w h
btn: 48 36
tex: 76 57
pvw: 156 88
*/
OutputPanel::OutputPanel(ParameterBagRef aParameterBag, TexturesRef aTexturesRef)
{
	mSetup = false;
	mParameterBag = aParameterBag;
	mTextures = aTexturesRef;

	//mVisible = false;

	setupParams();
}
void OutputPanel::setup()
{
	// spout
	g_Width = 640;
	g_Height = 480;
	// Set up the texture we will use to send out
	// We grab the screen so it has to be the same size
	spoutTexture = gl::Texture(g_Width, g_Height);
	strcpy_s(SenderName, "Reymenta Shader Sender"); // we have to set a sender name first
	spoutsender.SetDX9(true);
	// Optionally test for texture share compatibility
	// bMemoryMode informs us whether Spout initialized for texture share or memory share
	bMemoryMode = spoutsender.GetMemoryShareMode();// returns false
	// Initialize a sender
	bInitialized = spoutsender.CreateSender(SenderName, g_Width, g_Height);
	gl::enableDepthRead();
	gl::enableDepthWrite();

	mSpoutFbo = gl::Fbo(g_Width, g_Height);

	mSetup = true;
}
OutputPanelRef OutputPanel::create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef)
{
	return shared_ptr<OutputPanel>(new OutputPanel(aParameterBag, aTexturesRef));
}

void OutputPanel::setupParams()
{
	mParams = UIController::create("{ \"visible\":true, \"x\":874, \"y\":176, \"width\":180, \"height\":276, \"depth\":201, \"panelColor\":\"0x44282828\" }");
	// set custom fonts for a UIController
	mParams->setFont("label", mParameterBag->mLabelFont);
	mParams->setFont("smallLabel", mParameterBag->mSmallLabelFont);
	mParams->setFont("icon", mParameterBag->mIconFont);
	mParams->setFont("header", mParameterBag->mHeaderFont);
	mParams->setFont("body", mParameterBag->mBodyFont);
	mParams->setFont("footer", mParameterBag->mFooterFont);

	labelOutput = mParams->addLabel("Render", "{  \"width\":148 }");
	mParams->addButton("Spout\noutput", std::bind(&OutputPanel::toggleSendToOutput, this, std::placeholders::_1), "{ \"clear\":false, \"width\":72, \"stateless\":false }");
	mParams->addButton("Direct\nRender", std::bind(&OutputPanel::toggleDirectRender, this, std::placeholders::_1), "{ \"width\":72, \"stateless\":false }");
	mParams->addLabel("Rndr\nwin", "{ \"width\":48, \"clear\":false }");
	mParams->addButton("1", std::bind(&OutputPanel::createRenderWindow, this, 1, std::placeholders::_1), "{ \"clear\":false, \"stateless\":false, \"group\":\"render\", \"exclusive\":true }");
	mParams->addButton("x", std::bind(&OutputPanel::deleteRenderWindows, this, std::placeholders::_1), "{ \"stateless\":false, \"group\":\"render\", \"exclusive\":true, \"pressed\":true }");
	labelOutputResoXY = mParams->addLabel("OutputResoXY", "{  \"width\":" + toString(mParameterBag->mPreviewWidth) + " }");
	string reso = toString(mParameterBag->mRenderWidth*2)+ ", \"maxY\":0.0, \"minY\":" + toString(mParameterBag->mRenderHeight) + ", \"width\":" + toString(mParameterBag->mPreviewWidth) + " }";
	sliderOutputResoXY = mParams->addSlider2D("outputResoXY", &mParameterBag->mOutputResolution, "{ \"minX\":10.0, \"maxX\":" + toString(mParameterBag->mRenderWidth * 2) + ", \"maxY\":0.0, \"minY\":" + toString(mParameterBag->mRenderHeight) + ", \"width\":" + toString(mParameterBag->mPreviewWidth) + " }");
}
void OutputPanel::toggleDirectRender(const bool &pressed)
{
	mParameterBag->mDirectRender = pressed;
}
void OutputPanel::toggleSendToOutput(const bool &pressed)
{
	if (!isSetup()) setup();

	mParameterBag->mSendToOutput = pressed;
}
void OutputPanel::update()
{
	// animation
	if (mVisible)
	{
		mParams->update();
		if (!mParameterBag->mOptimizeUI)
		{
			labelOutputResoXY->setName(toString(floor(mParameterBag->mOutputResolution.x)) + "x" + toString(floor(mParameterBag->mOutputResolution.y)));
			sliderOutputResoXY->setBackgroundTexture(mSpoutFbo.getTexture());
		}
	}
}
void OutputPanel::draw()
{
	unsigned int width, height;
	if (mVisible)
	{
		// normal alpha blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		mParams->draw();
		// needed because of what the ping pong fbo is doing, at least
		gl::disableAlphaBlending();
	}
	// -------- SPOUT -------------
	if (bInitialized && mParameterBag->mSendToOutput) {
		width = g_Width; 
		height = g_Height;

		if (width != mParameterBag->mOutputResolution.x || height != mParameterBag->mOutputResolution.y) {
			// The sender dimensions have changed
			// Update the global width and height
			g_Width = mParameterBag->mOutputResolution.x;
			g_Height = mParameterBag->mOutputResolution.y;
			// TODO? Update the local texture to receive the new dimensions
			//RTE mSpoutFbo = gl::Fbo(g_Width, g_Height); 
			//return; // quit for next round
		}
		mSpoutFbo.bindFramebuffer();

		gl::setViewport(mSpoutFbo.getBounds());

		// clear the FBO
		gl::clear();

		gl::setMatricesWindow(mParameterBag->mOutputResolution.x, mParameterBag->mOutputResolution.y, mParameterBag->mOriginUpperLeft);
		gl::draw(mTextures->getFboTexture(mParameterBag->mMode));

		// stop drawing into the FBO
		mSpoutFbo.unbindFramebuffer();
		spoutsender.SendTexture(mSpoutFbo.getTexture().getId(), mSpoutFbo.getTexture().getTarget(), mSpoutFbo.getWidth(), mSpoutFbo.getHeight(), false);
	}
}

void OutputPanel::resize()
{
	mParams->resize();
}
void OutputPanel::shutdown()
{
	spoutsender.ReleaseSender();
}

void OutputPanel::show()
{
	mVisible = true;
}

void OutputPanel::hide()
{
	mVisible = false;
}

void OutputPanel::releaseGroup(const std::string &aGroup)
{
	mParams->releaseGroup(aGroup);
}

