/**
* \file Textures.h
* \brief Texture manager.
* \author Bruce LANE
* \version 0.1
* \date 13 november 2014
*
* Manages the textures.
*
*/
#pragma once

#include <string>
#include <vector>

#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "Logger.h"
#include "cinder/Utilities.h"
#include "cinder/Filesystem.h"
#include "cinder/Capture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"
// parameters
#include "ParameterBag.h"
// shaders
#include "Shaders.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta
{
	// stores the pointer to the Textures instance
	typedef std::shared_ptr<class Textures> TexturesRef;
	typedef struct cmonudcb {
		char name[256];
		unsigned int width, height;
		bool active;
	};

	class Textures {
	public:		
		Textures(ParameterBagRef aParameterBag, ShadersRef aShadersRef);
		static TexturesRef	create(ParameterBagRef aParameterBag, ShadersRef aShadersRef)
		{
			return shared_ptr<Textures>(new Textures(aParameterBag, aShadersRef));
		}
		void						setTextureFromFile(int index, string fileName);
		//! Returns Texture at index
		ci::gl::TextureRef			getTexture(int index);
		ci::gl::TextureRef			getMixTexture(int index);
		ci::gl::TextureRef			getFboTexture(int index);
		int							getTextureCount() { return sTextures.size(); };

		//! load image file as texture at index
		void						loadImageFile(int index, string aFile);

		//! main draw for fbos and textures
		void						draw();
		void						shutdown();
		void						setTextureSize( int index, int width, int height );
		void						flipMixFbo(bool flip);

	private:
		//! Logger
		LoggerRef					log;	
		// vectors of textures
		//! sTextures: Spout received textures
		vector<ci::gl::TextureRef>	sTextures;
		//! mixTextures: mix of 2 textures from sTextures or shaders
		vector<ci::gl::TextureRef>	mixTextures;
		//! check if valid index
		int checkedIndex(int index);
		//! parameters
		ParameterBagRef				mParameterBag;
		//! fbo
		vector<gl::FboRef>			mFbos;
		//! Shaders
		ShadersRef					mShaders;
		//! shader
		gl::GlslProgRef				aShader;

		vector<cmonudcb>			inputTextures;
	};
}