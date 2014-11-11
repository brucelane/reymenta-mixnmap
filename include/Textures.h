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

	class Textures {
	public:		
		Textures(ParameterBagRef aParameterBag, ShadersRef aShadersRef);
		static TexturesRef	create(ParameterBagRef aParameterBag, ShadersRef aShadersRef)
		{
			return shared_ptr<Textures>(new Textures(aParameterBag, aShadersRef));
		}
		void						setTextureFromFile(int index, string fileName);
		//void						setTexture(int index);
		ci::gl::TextureRef			getTexture(int index);
		ci::gl::TextureRef			getMixTexture(int index);
		ci::gl::TextureRef			getFboTexture(int index);
		int							getTextureCount() { return sTextures.size(); };

		// image
		void						loadImageFile( int index, string aFile );

		void						update();
		void						draw();
		void						shutdown();
		void						setTextureSize( int index, int width, int height );
		void						flipMixFbo(bool flip);

	private:
		// Logger
		LoggerRef					log;	
		// vectors of textures
		// sTextures: Spout received textures
		vector<ci::gl::TextureRef>	sTextures;
		// mixTextures: mix of 2 textures from sTextures or shaders
		vector<ci::gl::TextureRef>	mixTextures;

		// parameters
		ParameterBagRef				mParameterBag;
		// fbo
		vector<gl::FboRef>			mFbos;
		// Shaders
		ShadersRef					mShaders;
		// shaders
		gl::GlslProgRef				aShader;


	};
}