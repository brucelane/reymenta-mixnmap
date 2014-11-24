/**
* \file Textures.h
* \author Bruce LANE
* \date 20 november 2014
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
#include "cinder/Utilities.h"
#include "cinder/Filesystem.h"
#include "cinder/Capture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/VboMesh.h"

// parameters
#include "ParameterBag.h"
// shaders
#include "Shaders.h"
// log to file
#include "Logger.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta
{
	// stores the pointer to the Textures instance
	typedef std::shared_ptr<class Textures> TexturesRef;

	//! struct to keep track of the texture names for spout senders and shader fbo-rendered textures
	//typedef 
	struct Sender {
		char						SenderName[256];
		unsigned int				width, height;
		ci::gl::TextureRef			texture;
		bool						active;
	};
	struct ShadaFbo {
		ci::gl::FboRef				fbo;
		int							shadaIndex;
	};
	class Textures {
	public:		
		Textures(ParameterBagRef aParameterBag, ShadersRef aShadersRef);
		static TexturesRef			create(ParameterBagRef aParameterBag, ShadersRef aShadersRef)
		{
			return shared_ptr<Textures>(new Textures(aParameterBag, aShadersRef));
		}
		//! Returns Texture at index
		ci::gl::TextureRef			getTexture(int index);
		ci::gl::TextureRef			getMixTexture(int index);
		ci::gl::TextureRef			getFboTexture(int index);
		int							getInputTexturesCount() { return inputTextures.size(); };
		ci::gl::TextureRef			getSenderTexture(int index);
		int							createTexture(char name[256], unsigned int width, unsigned int height, gl::TextureRef texture);
		// from audio
		void						setAudioTexture(int audioTextureIndex, unsigned char *signal);

		//! load image file as texture
		void						setTextureFromFile(string fileName);

		//! main draw for fbos and textures
		void						draw();
		void						update();
		void						shutdown();
		void						setSenderTextureSize(int index, int width, int height);
		void						flipMixFbo(bool flip);
		char*						getSenderName(int index);
		void						renderShadersToFbo();
		void						renderMixesToFbo();
		void						saveThumb();
		void						setShadaIndex(int index) { selectedShada = index; };
		string						setInput(int index) { mShadaFbos[index].shadaIndex = selectedShada; return toString(selectedShada); };
		int							getShadaFbosSize() { return mShadaFbos.size(); };
		int							addShadaFbo();
	private:
		//! Logger
		LoggerRef					log;	
		// vectors of textures
		//! fboFormat
		gl::Fbo::Format				format;
		//! mixTextures: mix of 2 textures from sTextures or shaders
		vector<ci::gl::TextureRef>	mixTextures;
		//! check if valid index
		int							checkedIndex(int index);
		//! parameters
		ParameterBagRef				mParameterBag;
		//! mixes fbos
		vector<gl::FboRef>			mMixesFbos;
		//! shader fbos
		vector<ShadaFbo>			mShadaFbos;
		//! Shaders
		ShadersRef					mShaders;
		//! shader
		gl::GlslProgRef				aShader;
		int							selectedShada;
		//! inputTextures: vector of Spout received textures
		vector<Sender>				inputTextures;
		//! mesh for shader drawing
		gl::VboMeshRef				mMesh;

	};
}