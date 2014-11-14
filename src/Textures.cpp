/**
* \file Textures.cpp
* \brief Texture manager.
* \author Bruce LANE
* \version 0.1
* \date 13 november 2014
*
* Manages the textures.
*
*/
#include "Textures.h"

using namespace Reymenta;

Textures::Textures(ParameterBagRef aParameterBag, ShadersRef aShadersRef)
{
	mParameterBag = aParameterBag;
	mShaders = aShadersRef;
	// instanciate the logger class
	log = Logger::create("RenderTexturesLog.txt");
	log->logTimedString("Textures constructor");

	// init texture
	gl::TextureRef img = gl::Texture::create(loadImage(loadResource(IMG)));
	for (int i = 0; i < 8; i++)
	{
		sTextures.push_back(img);
		// TODO: replace with 0.jpg to 7.jpg if exists or load from settings file
		setTextureFromFile(i, "help.jpg");
	}
	// init 
	mixTextures.push_back(img);
	mFbos.push_back(gl::Fbo::create(mParameterBag->mFboWidth, mParameterBag->mFboHeight));//640x480
	//mFbos[0].getTexture(0).setFlipped(true);

	log->logTimedString("Textures constructor end");
}
void Textures::setTextureSize(int index, int width, int height)
{
	sTextures[index] = gl::Texture::create(width, height);
}
int Textures::checkedIndex(int index)
{
	int i = min(index, (int) sTextures.size() - 1);
	return max(i, 0);
}

void Textures::setTextureFromFile(int index, string fileName)
{
	if (index > sTextures.size() - 1) index = sTextures.size() - 1;
	if (index > 0)
	{
		try
		{
			string pathToAssetFile = (getAssetPath("") / fileName).string();

			if (!fs::exists(pathToAssetFile))
			{
				log->logTimedString("asset file not found: " + fileName);
			}
			else
			{
				log->logTimedString("asset found file: " + fileName);
				if (index < sTextures.size())
				{
					sTextures[index] = gl::Texture::create(loadImage(loadAsset(fileName)));
				}
				else
				{
					sTextures.push_back(gl::Texture::create(loadImage(loadAsset(fileName))));
				}
				log->logTimedString("asset loaded: " + fileName);
			}
		}
		catch (...)
		{
			log->logTimedString("Load asset error: " + fileName);
		}
	}
}

ci::gl::TextureRef Textures::getTexture(int index)
{
	return sTextures[checkedIndex(index)];
}
ci::gl::TextureRef Textures::getMixTexture(int index)
{
	if (index > mixTextures.size() - 1) index = mixTextures.size() - 1;
	return mixTextures[index];
}
ci::gl::TextureRef Textures::getFboTexture(int index)
{
	if (index > mFbos.size() - 1) index = mFbos.size() - 1;
	return mFbos[index]->getColorTexture();
}
void Textures::loadImageFile(int index, string aFile)
{
	try
	{
		// try loading image file
		sTextures[checkedIndex(index)] = gl::Texture::create(loadImage(aFile));
	}
	catch (...)
	{
		log->logTimedString("Error loading image:" + aFile);
	}
}
void Textures::flipMixFbo(bool flip)
{
	//mFbos[0].getTexture(0).setFlipped(flip);
	//mixTextures[0].setFlipped(flip);
	mParameterBag->mOriginUpperLeft = flip;
}

void Textures::draw()
{
	//! draw with mix.frag if 2 textures to mix
	//! loop if several active fbos to draw

	/***********************************************
	* mix 2 textures in FBOs with mix shader 
	*/
	// draw using the mix shader
	mFbos[0]->bindFramebuffer();

	// setup the viewport to match the dimensions of the FBO
	gl::ScopedViewport scpVp(ivec2(0), mFbos[0]->getSize());

	// clear the FBO
	gl::clear();
	gl::setMatricesWindow(mParameterBag->mFboWidth, mParameterBag->mFboHeight, mParameterBag->mOriginUpperLeft);

	aShader = mShaders->getMixShader();
	aShader->bind();
	aShader->uniform("iGlobalTime", mParameterBag->iGlobalTime);
	aShader->uniform("iResolution", vec3(mParameterBag->mFboWidth, mParameterBag->mFboHeight, 1.0));
	aShader->uniform("iMouse", vec4(mParameterBag->mRenderPosXY.x, mParameterBag->mRenderPosXY.y, mParameterBag->iMouse.z, mParameterBag->iMouse.z));//iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
	aShader->uniform("iChannel0", 0);
	aShader->uniform("iChannel1", 1);
	aShader->uniform("iColor", vec3(mParameterBag->controlValues[1], mParameterBag->controlValues[2], mParameterBag->controlValues[3]));
	aShader->uniform("iBackgroundColor", vec3(mParameterBag->controlValues[5], mParameterBag->controlValues[6], mParameterBag->controlValues[7]));
	aShader->uniform("iSteps", (int)mParameterBag->controlValues[16]);
	aShader->uniform("iRatio", mParameterBag->controlValues[11]);
	aShader->uniform("width", 1);
	aShader->uniform("height", 1);
	aShader->uniform("iRenderXY", mParameterBag->mRenderXY);
	aShader->uniform("iZoom", mParameterBag->controlValues[13]);
	aShader->uniform("iAlpha", mParameterBag->controlValues[4]);
	aShader->uniform("iRotationSpeed", mParameterBag->controlValues[19]);
	aShader->uniform("iCrossfade", mParameterBag->controlValues[15]);
	aShader->uniform("iPixelate", mParameterBag->controlValues[18]);
	aShader->uniform("iExposure", mParameterBag->controlValues[14]);
	aShader->uniform("iDeltaTime", mParameterBag->iDeltaTime);
	aShader->uniform("iFade", (int)mParameterBag->iFade);
	aShader->uniform("iToggle", (int)mParameterBag->controlValues[46]);
	aShader->uniform("iLight", (int)mParameterBag->iLight);
	aShader->uniform("iLightAuto", (int)mParameterBag->iLightAuto);
	aShader->uniform("iGreyScale", (int)mParameterBag->iGreyScale);
	aShader->uniform("iTransition", mParameterBag->iTransition);
	aShader->uniform("iAnim", mParameterBag->iAnim.value());
	aShader->uniform("iRepeat", (int)mParameterBag->iRepeat);
	aShader->uniform("iVignette", (int)mParameterBag->controlValues[47]);
	aShader->uniform("iInvert", (int)mParameterBag->controlValues[48]);
	aShader->uniform("iDebug", (int)mParameterBag->iDebug);
	aShader->uniform("iShowFps", (int)mParameterBag->iShowFps);
	aShader->uniform("iFps", mParameterBag->iFps);
	aShader->uniform("iTempoTime", mParameterBag->iTempoTime);
	aShader->uniform("iGlitch", (int)mParameterBag->controlValues[45]);

	sTextures[mParameterBag->iChannels[0]]->bind(0);
	sTextures[mParameterBag->iChannels[1]]->bind(1);
	gl::drawSolidRect(Rectf(0, 0, mParameterBag->mFboWidth, mParameterBag->mFboHeight));
	// stop drawing into the FBO
	mFbos[0]->unbindFramebuffer();

	sTextures[mParameterBag->iChannels[0]]->unbind();
	sTextures[mParameterBag->iChannels[1]]->unbind();

	//aShader->unbind();
	sTextures[2] = mFbos[0]->getColorTexture();
	mixTextures[0] = mFbos[0]->getColorTexture();
}

void Textures::shutdown()
{
	sTextures.clear();
}
