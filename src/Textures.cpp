/**
* \file Textures.cpp
* \author Bruce LANE
* \date 20 november 2014
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
	startupImage = gl::Texture::create(loadImage(loadResource(IMG)));

	// init
	string pathToStartupFile = (getAssetPath("") / "startup.jpg").string();
	if (fs::exists(pathToStartupFile))
	{
		log->logTimedString("startup.jpg image found, loading");
		startupImage = gl::Texture::create(loadImage(loadAsset("startup.jpg")));
		log->logTimedString("startup.jpg image loaded");
	}
	createTexture("startup image", mParameterBag->mFboWidth, mParameterBag->mFboHeight, startupImage);
	// create a rectangle to be drawn with our shader program
	// default is from -0.5 to 0.5, so we scale by 2 to get -1.0 to 1.0
	// coming soon in Cinder? mMesh = gl::VboMesh::create(geom::Rect().scale(vec2(2.0f, 2.0f))); 
	mMesh = gl::VboMesh::create(geom::Rect(Rectf(-2.0, -2.0, 2.0, 2.0)));
	selectedShada = 0;
	currentInputTextureIndex = 0;
	currentMode = 0;

	log->logTimedString("Textures constructor end");
}
void Textures::createWarpInput()
{
	WarpInput newWarpInput;
	newWarpInput.leftIndex = 0;
	newWarpInput.leftMode = 0;
	newWarpInput.rightIndex = 0;
	newWarpInput.rightMode = 0;
	warpInputs.push_back(newWarpInput);
	// init mixTextures
	//mixTextures.push_back(startupImage);
	mMixesFbos.push_back(gl::Fbo::create(mParameterBag->mFboWidth, mParameterBag->mFboHeight, format.depthTexture()));//640x360 or 480?
}

string Textures::setInput(int index, bool left) 
{ 
	string name;
	if (currentMode == 0)
	{
		// 0 = input texture mode
		name = "T" + toString(currentInputTextureIndex);
		if (left)
		{
			warpInputs[index].leftIndex = currentInputTextureIndex;
			warpInputs[index].leftMode = 0; // 0 for input texture
		}
		else
		{
			warpInputs[index].rightIndex = currentInputTextureIndex;
			warpInputs[index].rightMode = 0; // 0 for input texture
		}
	}
	else
	{
		// 1 = shader mode
		mShadaFbos[index].shadaIndex = selectedShada;
		name = "S" + toString(selectedShada);
		if (left)
		{
			warpInputs[index].leftIndex = selectedShada;
			warpInputs[index].leftMode = 1; // 1 for shader

		}
		else
		{
			warpInputs[index].rightIndex = selectedShada;
			warpInputs[index].rightMode = 1; // 1 for shader

		}
	}
	return name; 
};

ci::gl::TextureRef Textures::getSenderTexture(int index)
{
	return inputTextures[checkedIndex(index)].texture;
}
void Textures::setSenderTextureSize(int index, int width, int height)
{
	inputTextures[checkedIndex(index)].width = width;
	inputTextures[checkedIndex(index)].height = height;
	inputTextures[checkedIndex(index)].texture = gl::Texture::create(width, height);
}
int Textures::addShadaFbo()
{
	// add a ShadaFbo
	ShadaFbo sFbo;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	sFbo.fbo = gl::Fbo::create(mParameterBag->mFboWidth, mParameterBag->mFboHeight, format.depthTexture());
	sFbo.shadaIndex = 0;
	mShadaFbos.push_back(sFbo);
	return mShadaFbos.size() - 1;
}

int Textures::createTexture(char name[256], unsigned int width, unsigned int height, gl::TextureRef texture)
{
	Sender newTexture;
	// create new texture
	memcpy(&newTexture.SenderName[0], name, strlen(name) + 1);
	newTexture.width = width;
	newTexture.height = height;
	newTexture.texture = texture;
	//! add to the inputTextures vector
	inputTextures.push_back(newTexture);
	return inputTextures.size() - 1;
}
char* Textures::getSenderName(int index) 
{ 
	return &inputTextures[checkedIndex(index)].SenderName[0];
}

void Textures::setAudioTexture(int audioTextureIndex, unsigned char *signal)
{
	inputTextures[audioTextureIndex].texture = gl::Texture::create(signal, GL_LUMINANCE16I_EXT, 512, 2);
}
int Textures::checkedIndex(int index)
{
	int i = min(index, int(inputTextures.size() - 1));
	return max(i, 0);
}

void Textures::setTextureFromFile(string fileName)
{
	string shortName = "image";
	char *name;
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
			string mFile = fr.string();
			if (mFile.find_last_of("\\") != std::string::npos)
			{
				shortName = mFile.substr(mFile.find_last_of("\\") + 1);
			}
			name = &shortName[0];
			gl::TextureRef newTexture = gl::Texture::create(loadImage(fileName));
			createTexture(name, newTexture->getWidth(), newTexture->getHeight(), newTexture);
			log->logTimedString("asset loaded: " + fileName);
		}
	}
	catch (...)
	{
		log->logTimedString("Load asset error: " + fileName);
	}
}

void Textures::flipMixFbo(bool flip)
{
	//mFbos[0].getTexture(0).setFlipped(flip);
	//mixTextures[0].setFlipped(flip);
	mParameterBag->mOriginUpperLeft = flip;
}
void Textures::saveThumb()
{
	string filename = mShaders->getFragFileName() + ".jpg";
	try
	{
		mShadaFbos[mParameterBag->mCurrentShadaFboIndex].fbo->bindFramebuffer();
		// Should get the FBO's pixels since it is bound (instead of the screen's)
		Surface fboSurf = copyWindowSurface(Area(ivec2(0,0),ivec2(mParameterBag->mFboWidth,mParameterBag->mFboHeight)));  
		mShadaFbos[mParameterBag->mCurrentShadaFboIndex].fbo->unbindFramebuffer();
		fs::path path = getAssetPath("") / "thumbs" / filename;
		writeImage(path, ImageSourceRef(fboSurf));
		log->logTimedString("saved:" + filename);
		int i = 0;
		for (auto &mFbo : mShadaFbos)
		{
			filename = mShaders->getFragFileName() + static_cast<ostringstream*>(&(ostringstream() << i))->str() + ".jpg";
			mFbo.fbo->bindFramebuffer();
			// Should get the FBO's pixels since it is bound (instead of the screen's)
			Surface fboSurf = copyWindowSurface(Area(ivec2(0,0),ivec2(mParameterBag->mFboWidth,mParameterBag->mFboHeight)));  
			mFbo.fbo->unbindFramebuffer();
			fs::path path = getAssetPath("") / "thumbs" / filename;
			writeImage(path, ImageSourceRef(fboSurf));


			i++;
		}
	}
	catch (const std::exception &e)
	{
		log->logTimedString("unable to save:" + filename + string(e.what()));
	}
}

void Textures::update()
{

}
ci::gl::TextureRef Textures::getTexture(int index)
{
	return inputTextures[checkedIndex(index)].texture;
}
/*ci::gl::TextureRef Textures::getMixTexture(int index)
{
	if (index > mixTextures.size() - 1) index = mixTextures.size() - 1;
	return mixTextures[index];
}*/
ci::gl::TextureRef Textures::getFboTexture(int index)
{
	if (index > mShadaFbos.size() - 1) index = mShadaFbos.size() - 1;
	return mShadaFbos[index].fbo->getColorTexture();
}
void Textures::renderShadersToFbo()
{
	for (auto &mFbo : mShadaFbos)
	{
		// this will restore the old framebuffer binding when we leave this function
		// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
		// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
		gl::ScopedFramebuffer fbScp(mFbo.fbo);
		// clear out the FBO with black
		gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));

		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0.0), mFbo.fbo->getSize());

		gl::ScopedGlslProg shader(mShaders->getShader(mFbo.shadaIndex));
		// draw our screen rectangle
		gl::draw(mMesh);
	}
}
void Textures::renderMixesToFbo()
{
	int i = 0;
	for (auto &mFbo : mMixesFbos)
	{
		// this will restore the old framebuffer binding when we leave this function
		// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
		// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
		gl::ScopedFramebuffer fbScp(mFbo);
		// clear out the FBO with black
		gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));

		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0.0), mFbo->getSize());

		gl::ScopedGlslProg shader(mShaders->getMixShader());
		if (warpInputs[i].leftMode == 0)
		{
			// 0 for input texture
			getSenderTexture(warpInputs[i].leftIndex)->bind(0);
		}
		else
		{
			// 1 for shader
			getFboTexture(warpInputs[i].leftIndex)->bind(0);
		}
		if (warpInputs[i].rightMode == 0)
		{
			// 0 for input texture
			getSenderTexture(warpInputs[i].rightIndex)->bind(1);
		}
		else
		{
			// 1 for shader
			getFboTexture(warpInputs[i].rightIndex)->bind(1);
		}

		// draw our screen rectangle
		gl::draw(mMesh);
		i++;
	}
}
void Textures::draw()
{
	//! 1 render the active shaders to Fbos
	renderShadersToFbo();
	//! 2 render mixes of shader Fbos as texture, images, Spout sources as Fbos
	renderMixesToFbo();
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
