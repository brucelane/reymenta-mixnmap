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

	// init
	mNewSenderName[0] = NULL;
	gl::Fbo::Format format;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	for (int i = 0; i < MAX; i++)
	{	
		// create inputTextures and init with static image
		memcpy(&inputTextures[i].SenderName[0], mNewSenderName, strlen(mNewSenderName) + 1);
		inputTextures[i].width = mParameterBag->mFboWidth;
		inputTextures[i].height = mParameterBag->mFboHeight;
		// TODO: replace with 0.jpg to 7.jpg if exists or load from settings file
		inputTextures[i].texture = img;
		// init mixTextures
		mixTextures.push_back(img);
		// create FBO
		mFbos.push_back(gl::Fbo::create(mParameterBag->mFboWidth, mParameterBag->mFboHeight, format.depthTexture()));//640x480
	}
	// create a rectangle to be drawn with our shader program
	// default is from -0.5 to 0.5, so we scale by 2 to get -1.0 to 1.0
	mMesh = gl::VboMesh::create(geom::Rect());// .scale(vec2(2.0f, 2.0f)));

	log->logTimedString("Textures constructor end");
}
ci::gl::TextureRef Textures::getSenderTexture(int index)
{
	return inputTextures[index].texture;
}
void Textures::setSenderTextureSize(int index, int width, int height)
{
	inputTextures[index].width = width;
	inputTextures[index].height = height;
	inputTextures[index].texture = gl::Texture::create(width, height);
}
void Textures::setAudioTexture( unsigned char *signal)
{
	memcpy(audioTexture.SenderName, "Audio", strlen("Audio") + 1);
	audioTexture.width = 512;
	audioTexture.height = 2;
	audioTexture.texture = gl::Texture::create(signal, GL_LUMINANCE16I_EXT, 512, 2);
}
int Textures::checkedIndex(int index)
{
	int i = min(index, MAX - 1);
	return max(i, 0);
}

void Textures::setTextureFromFile(int index, string fileName)
{
	if (index > MAX - 1) index = MAX - 1;
	if (index > 0)
	{
		try
		{
			if (!fs::exists(fileName))
			{
				log->logTimedString("asset file not found: " + fileName);
			}
			else
			{
				log->logTimedString("asset found file: " + fileName);
				fs::path fr = fileName;
				string name = "image";
				string mFile = fr.string();
				if (mFile.find_last_of("\\") != std::string::npos) name = mFile.substr(mFile.find_last_of("\\") + 1);

				memcpy(inputTextures[index].SenderName, name.c_str(), strlen(name.c_str()) + 1);
				inputTextures[index].texture = gl::Texture::create(loadImage(fileName));
				inputTextures[index].width = inputTextures[index].texture->getWidth();
				inputTextures[index].height = inputTextures[index].texture->getHeight();

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
	return inputTextures[checkedIndex(index)].texture;
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

void Textures::flipMixFbo(bool flip)
{
	//mFbos[0].getTexture(0).setFlipped(flip);
	//mixTextures[0].setFlipped(flip);
	mParameterBag->mOriginUpperLeft = flip;
}

void Textures::update()
{
}
void Textures::renderToFbo()
{
	int i = 0;
	for (auto &mFbo : mFbos)
	{
		// this will restore the old framebuffer binding when we leave this function
		// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
		// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
		gl::ScopedFramebuffer fbScp(mFbo);
		// clear out the FBO with blue
		gl::clear(Color(0.25, 0.5f, 1.0f));

		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0.0), mFbo->getSize());

		gl::ScopedGlslProg shader(mShaders->getShader(i));
		// draw our screen rectangle
		gl::draw(mMesh);
		i++;
	}
}
void Textures::draw()
{
	renderToFbo();
	/**********************************************
	* library FBOs
	
	// start of mLibraryFbos[mParameterBag->mLeftFboIndex]
	mFbos[mParameterBag->mLeftFboIndex]->bindFramebuffer();
	// setup the viewport to match the dimensions of the FBO
	//gl::ScopedViewport scpVp0(ivec2(0), mFbos[mParameterBag->mLeftFboIndex]->getSize());
	gl::pushMatrices();
	gl::pushViewport(0, 0, mParameterBag->mFboWidth, mParameterBag->mFboHeight);

	// clear the FBO
	gl::clear(Color(mParameterBag->controlValues[5], mParameterBag->controlValues[6], mParameterBag->controlValues[7]));
	gl::setMatricesWindow(mParameterBag->mFboWidth, mParameterBag->mFboHeight);
	//gl::setMatricesWindow(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight, mParameterBag->mOriginUpperLeft);

	aShader = mShaders->getShader(mParameterBag->mLeftFragIndex);
	aShader->bind();
	aShader->uniform("iGlobalTime", mParameterBag->iGlobalTime);
	//aShader->uniform("iResolution", vec3(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight, 1.0));
	aShader->uniform("iResolution", vec3(mParameterBag->mFboWidth, mParameterBag->mFboHeight, 1.0));
	aShader->uniform("iChannelResolution", mParameterBag->iChannelResolution, 4);
	aShader->uniform("iMouse", vec4(mParameterBag->mRenderPosXY.x, mParameterBag->mRenderPosXY.y, mParameterBag->iMouse.z, mParameterBag->iMouse.z));//iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
	aShader->uniform("iChannel0", mParameterBag->iChannels[0]);
	aShader->uniform("iChannel1", mParameterBag->iChannels[1]);
	aShader->uniform("iChannel2", mParameterBag->iChannels[2]);
	aShader->uniform("iChannel3", mParameterBag->iChannels[3]);
	aShader->uniform("iChannel4", mParameterBag->iChannels[4]);
	aShader->uniform("iChannel5", mParameterBag->iChannels[5]);
	aShader->uniform("iChannel6", mParameterBag->iChannels[6]);
	aShader->uniform("iChannel7", mParameterBag->iChannels[7]);
	aShader->uniform("iAudio0", 0);
	aShader->uniform("iFreq0", mParameterBag->iFreqs[0]);
	aShader->uniform("iFreq1", mParameterBag->iFreqs[1]);
	aShader->uniform("iFreq2", mParameterBag->iFreqs[2]);
	aShader->uniform("iFreq3", mParameterBag->iFreqs[3]);
	aShader->uniform("iChannelTime", mParameterBag->iChannelTime, 4);
	aShader->uniform("iColor", vec3(mParameterBag->controlValues[1], mParameterBag->controlValues[2], mParameterBag->controlValues[3]));// mParameterBag->iColor);
	aShader->uniform("iBackgroundColor", vec3(mParameterBag->controlValues[5], mParameterBag->controlValues[6], mParameterBag->controlValues[7]));// mParameterBag->iBackgroundColor);
	aShader->uniform("iSteps", (int)mParameterBag->controlValues[16]);
	aShader->uniform("iRatio", mParameterBag->controlValues[11]);//check if needed: +1;//mParameterBag->iRatio); 
	aShader->uniform("width", 1);
	aShader->uniform("height", 1);
	aShader->uniform("iRenderXY", mParameterBag->mLeftRenderXY);
	aShader->uniform("iZoom", mParameterBag->iZoomLeft);
	aShader->uniform("iAlpha", mParameterBag->controlValues[4]);
	aShader->uniform("iBlendmode", (int)mParameterBag->controlValues[15]);
	aShader->uniform("iRotationSpeed", mParameterBag->controlValues[19]);
	aShader->uniform("iCrossfade", mParameterBag->iPreviewCrossfade);
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

	for (size_t m = 0; m < 2; m++)
	{
		getTexture(m)->bind(m);
	}
	gl::drawSolidRect(Rectf(0, 0, mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight));
	// stop drawing into the FBO
	mFbos[mParameterBag->mLeftFboIndex]->unbindFramebuffer();

	for (size_t m = 0; m < 2; m++)
	{
		getTexture(m)->unbind();
	}
	gl::popViewport();
	gl::popMatrices();
	//aShader->unbind();
	inputTextures[mParameterBag->iChannels[0]].texture = mFbos[mParameterBag->mLeftFboIndex]->getColorTexture();
	// end of mLibraryFbos[mParameterBag->mLeftFboIndex]

	// start of mLibraryFbos[mParameterBag->mRightFboIndex]
	mFbos[mParameterBag->mRightFboIndex]->bindFramebuffer();
	// setup the viewport to match the dimensions of the FBO
	//gl::ScopedViewport scpVp1(ivec2(0), mFbos[mParameterBag->mRightFboIndex]->getSize());
	gl::pushMatrices();
	gl::pushViewport(0, 0, mParameterBag->mFboWidth, mParameterBag->mFboHeight);

	// clear the FBO
	gl::clear(Color(mParameterBag->controlValues[5], mParameterBag->controlValues[6], mParameterBag->controlValues[7]));
	gl::setMatricesWindow(mParameterBag->mFboWidth, mParameterBag->mFboHeight);
	//gl::setMatricesWindow(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight, mParameterBag->mOriginUpperLeft);

	aShader = mShaders->getShader(mParameterBag->mRightFragIndex);
	aShader->bind();
	aShader->uniform("iGlobalTime", mParameterBag->iGlobalTime);
	//aShader->uniform("iResolution", vec3(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight, 1.0));
	aShader->uniform("iResolution", vec3(mParameterBag->mFboWidth, mParameterBag->mFboHeight, 1.0));
	aShader->uniform("iChannelResolution", mParameterBag->iChannelResolution, 4);
	aShader->uniform("iMouse", vec4(mParameterBag->mRenderPosXY.x, mParameterBag->mRenderPosXY.y, mParameterBag->iMouse.z, mParameterBag->iMouse.z));//iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
	aShader->uniform("iChannel0", mParameterBag->iChannels[0]);
	aShader->uniform("iChannel1", mParameterBag->iChannels[1]);
	aShader->uniform("iChannel2", mParameterBag->iChannels[2]);
	aShader->uniform("iChannel3", mParameterBag->iChannels[3]);
	aShader->uniform("iChannel4", mParameterBag->iChannels[4]);
	aShader->uniform("iChannel5", mParameterBag->iChannels[5]);
	aShader->uniform("iChannel6", mParameterBag->iChannels[6]);
	aShader->uniform("iChannel7", mParameterBag->iChannels[7]);
	aShader->uniform("iAudio0", 0);
	aShader->uniform("iFreq0", mParameterBag->iFreqs[0]);
	aShader->uniform("iFreq1", mParameterBag->iFreqs[1]);
	aShader->uniform("iFreq2", mParameterBag->iFreqs[2]);
	aShader->uniform("iFreq3", mParameterBag->iFreqs[3]);
	aShader->uniform("iChannelTime", mParameterBag->iChannelTime, 4);
	aShader->uniform("iColor", vec3(mParameterBag->controlValues[1], mParameterBag->controlValues[2], mParameterBag->controlValues[3]));// mParameterBag->iColor);
	aShader->uniform("iBackgroundColor", vec3(mParameterBag->controlValues[5], mParameterBag->controlValues[6], mParameterBag->controlValues[7]));// mParameterBag->iBackgroundColor);
	aShader->uniform("iSteps", (int)mParameterBag->controlValues[16]);
	aShader->uniform("iRatio", mParameterBag->controlValues[11]);//check if needed: +1;//mParameterBag->iRatio); 
	aShader->uniform("width", 1);
	aShader->uniform("height", 1);
	aShader->uniform("iRenderXY", mParameterBag->mRightRenderXY);
	aShader->uniform("iZoom", mParameterBag->iZoomRight);
	aShader->uniform("iAlpha", mParameterBag->controlValues[4]);
	aShader->uniform("iBlendmode", (int)mParameterBag->controlValues[15]);
	aShader->uniform("iRotationSpeed", mParameterBag->controlValues[19]);
	aShader->uniform("iCrossfade", mParameterBag->iPreviewCrossfade);
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

	for (size_t m = 0; m < 2; m++)
	{
		getTexture(m)->bind(m);
	}
	gl::drawSolidRect(Rectf(0, 0, mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight));
	// stop drawing into the FBO
	mFbos[mParameterBag->mRightFboIndex]->unbindFramebuffer();

	for (size_t m = 0; m < 2; m++)
	{
		getTexture(m)->unbind();
	}
	gl::popViewport();
	gl::popMatrices();

	//aShader->unbind();
	inputTextures[mParameterBag->iChannels[1]].texture = mFbos[mParameterBag->mRightFboIndex]->getColorTexture();

	// end of mLibraryFbos[mParameterBag->mRightFboLibraryIndex]


	//! draw with mix.frag if 2 textures to mix
	//! loop if several active fbos to draw

	/***********************************************
	* mix 2 textures in FBOs with mix shader 
	
	// draw using the mix shader
	mFbos[0]->bindFramebuffer();

	// setup the viewport to match the dimensions of the FBO
	//gl::ScopedViewport scpVp2(ivec2(0), mFbos[0]->getSize());
	gl::pushMatrices();
	gl::pushViewport(0, 0, mParameterBag->mFboWidth, mParameterBag->mFboHeight);

	// clear the FBO
	gl::clear();
	gl::setMatricesWindow(mParameterBag->mFboWidth, mParameterBag->mFboHeight);
	//gl::setMatricesWindow(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight, mParameterBag->mOriginUpperLeft);

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

	inputTextures[mParameterBag->iChannels[0]].texture->bind(0);
	inputTextures[mParameterBag->iChannels[1]].texture->bind(1);
	gl::drawSolidRect(Rectf(0, 0, mParameterBag->mFboWidth, mParameterBag->mFboHeight));
	// stop drawing into the FBO
	mFbos[0]->unbindFramebuffer();

	inputTextures[mParameterBag->iChannels[0]].texture->unbind();
	inputTextures[mParameterBag->iChannels[1]].texture->unbind();
	gl::popViewport();
	gl::popMatrices();

	mixTextures[0] = mFbos[0]->getColorTexture();*/
}

void Textures::shutdown()
{
	//TODO for inputTextures sTextures.clear();
}
