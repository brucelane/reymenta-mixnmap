#include "WarpWrapper.h"

using namespace Reymenta;

WarpWrapper::WarpWrapper(ParameterBagRef aParameterBag, TexturesRef aTexturesRef, ShadersRef aShadersRef)
{
	mParameterBag = aParameterBag;
	mShaders = aShadersRef;
	mTextures = aTexturesRef;
	// instanciate the logger class
	log = Logger::create("WarpWrapperLog.txt");
	log->logTimedString("WarpWrapper constructor");

	//mUseBeginEnd = false;
	//mWarps.push_back(WarpPerspectiveBilinear::create());
	// initialize warps
	log->logTimedString("Loading MixnMapWarps.xml");
	fs::path settings = getAssetPath("") / warpsFileName;
	if (fs::exists(settings))
	{
		// load warp settings from file if one exists
		mWarps = Warp::readSettings(loadFile(settings));
	}
	else
	{
		// otherwise create a warp from scratch
		mWarps.push_back(WarpPerspectiveBilinear::create());
	}

	// adjust the content size of the warps
	Warp::setSize(mWarps, mTextures->getTexture(0)->getSize());
	log->logTimedString("Warps count " + toString(mWarps.size()));

	gl::enableDepthRead();
	gl::enableDepthWrite();
}
void WarpWrapper::save()
{
	log->logTimedString("Saving MixnMapWarps.xml");
	fs::path settings = getAssetPath("") / warpsFileName;
	Warp::writeSettings(mWarps, writeFile(settings));
}
void WarpWrapper::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize(mWarps);

}
void WarpWrapper::mouseMove(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseMove(mWarps, event))
	{
		// let your application perform its mouseMove handling here
	}
}
void WarpWrapper::mouseDown(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDown(mWarps, event))
	{
		// let your application perform its mouseDown handling here
	}
}
void WarpWrapper::mouseDrag(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDrag(mWarps, event))
	{
		// let your application perform its mouseDrag handling here
	}
}
void WarpWrapper::mouseUp(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseUp(mWarps, event))
	{
		// let your application perform its mouseUp handling here
	}
}
void WarpWrapper::keyDown(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyDown(mWarps, event))
	{
		// warp editor did not handle the key, so handle it here
		switch (event.getCode())
		{
		case KeyEvent::KEY_n:
			// create a warp
			mWarps.push_back(WarpPerspectiveBilinear::create());
			break;
		case KeyEvent::KEY_f:
			// toggle full screen
			setFullScreen(!isFullScreen());
			break;
		case KeyEvent::KEY_w:
			// toggle warp edit mode
			Warp::enableEditMode(!Warp::isEditModeEnabled());
			break;
		}
	}
}

void WarpWrapper::keyUp(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyUp(mWarps, event))
	{
		// let your application perform its keyUp handling here
	}
}
void WarpWrapper::draw()
{

	gl::setMatricesWindow(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight, mParameterBag->mOriginUpperLeft);
	//TODO? mViewportArea = Area(0, 0, mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);

	//TODO? gl::setViewport(mViewportArea);
	// clear the window and set the drawing color to white
	gl::clear();
	gl::color(Color::white());
	gl::disableAlphaBlending();
	gl::disable(GL_TEXTURE_2D);

	int i = 0;
	// iterate over the warps and draw their content
	for (WarpConstIter itr = mWarps.begin(); itr != mWarps.end(); ++itr)
	{
		// create a readable reference to our warp, to prevent code like this: (*itr)->begin();
		WarpRef warp(*itr);

		//if (i < mParameterBag->mWarpCount)
		//{
			warp->draw(mTextures->getFboTexture(mParameterBag->iWarpFboChannels[i]), mTextures->getFboTexture(mParameterBag->iWarpFboChannels[i])->getBounds());
			//warp->draw(mTextures->getMixTexture(mParameterBag->iWarpFboChannels[i]), mTextures->getMixTexture(mParameterBag->iWarpFboChannels[i])->getBounds());
			//warp->draw(mTextures->getTexture(mParameterBag->iWarpFboChannels[i]), mTextures->getSenderTexture(mParameterBag->iWarpFboChannels[i])->getBounds());
		//}

		i++;
	}
}
WarpWrapper::~WarpWrapper()
{
	log->logTimedString("WarpWrapper destructor");
}


