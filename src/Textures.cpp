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
	selectedInputTexture = 0;
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
	newWarpInput.iCrossfade = 0.5;
	warpInputs.push_back(newWarpInput);
	// init mixTextures
	mMixesFbos.push_back(gl::Fbo::create(mParameterBag->mFboWidth, mParameterBag->mFboHeight, fboFormat.depthTexture()));//640x360 or 480?
}
void Textures::setShadaIndex(int index) 
{ 
	currentMode = 1; 
	log->logTimedString("setShadaIndex:" + toString(index)); 
	selectedShada = index; 
}
void Textures::setInputTextureIndex(int index) 
{ 
	currentMode = 0; 
	log->logTimedString("setInputTextureIndex:" + toString(index)); 
	selectedInputTexture = index; 
}

WarpInput Textures::setInput(int index, bool left)
{ 
	string name;
	if (currentMode == 0)
	{
		// 0 = input texture mode
		name = "T" + toString(selectedInputTexture);
		if (left)
		{
			warpInputs[index].leftIndex = selectedInputTexture;
			warpInputs[index].leftMode = 0; // 0 for input texture
		}
		else
		{
			warpInputs[index].rightIndex = selectedInputTexture;
			warpInputs[index].rightMode = 0; // 0 for input texture
		}
	}
	else
	{
		// 1 = shader mode
		//??? mShadaFbos[index].shadaIndex = selectedShada;
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
	return warpInputs[index];
};

int Textures::addShadaFbo()
{
	// add a ShadaFbo
	ShadaFbo sFbo;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	sFbo.fbo = gl::Fbo::create(mParameterBag->mFboWidth, mParameterBag->mFboHeight, fboFormat.depthTexture());
	sFbo.shadaIndex = mShadaFbos.size();
	mShadaFbos.push_back(sFbo);
	return mShadaFbos.size() - 1;
}

int Textures::createSpoutTexture(char name[256], unsigned int width, unsigned int height)
{
	Sender newTexture;
	// create new texture
	memcpy(&newTexture.SenderName[0], name, strlen(name) + 1);
	newTexture.width = width;
	newTexture.height = height;
	newTexture.texture = gl::Texture::create(width, height);
	//! add to the inputTextures vector
	inputTextures.push_back(newTexture);
	return inputTextures.size() - 1;
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
ci::gl::TextureRef Textures::getMixTexture(int index)
{
	if (index > mMixesFbos.size() - 1) index = mMixesFbos.size() - 1;
	return mMixesFbos[index]->getColorTexture();
}
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
		getFboTexture(0)->bind(0);
		getSenderTexture(1)->bind(1);
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
		mShaders->getMixShader()->uniform("iCrossfade", warpInputs[i].iCrossfade);
		//warpInputs[i].iCrossfade += 0.1;
		//if (warpInputs[i].iCrossfade > 1.0) warpInputs[i].iCrossfade = 0.0;
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
}

void Textures::shutdown()
{
	//TODO for inputTextures sTextures.clear();
}
