#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Utilities.h"
#include "cinder/Timeline.h"

#include "Resources.h"
// log
#include "Logger.h"
// parameters
#include "ParameterBag.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta
{
	// stores the pointer to the Shaders instance
	typedef std::shared_ptr<class Shaders> ShadersRef;

	class Shaders {
	public:		
		Shaders( ParameterBagRef aParameterBag );
		virtual					~Shaders();
		static ShadersRef	create( ParameterBagRef aParameterBag )
		{
			return shared_ptr<Shaders>( new Shaders( aParameterBag ) );
		}

		string getFragError();
		gl::GlslProgRef getShader(int aIndex) { return mFragmentShaders[aIndex]; };
		gl::GlslProgRef getMixShader() { return mMixShader; };
		bool loadPixelFragmentShader(string aFilePath);
		string getFragFileName() { return mFragFileName; };
		bool setGLSLString(string pixelFrag);

		/*void loadFragmentShader(boost::filesystem::path aFilePath);
		string getFileName(string aFilePath);
		string getNewFragFileName( string aFilePath);
		string getFragStringFromFile( string fileName );*/
	private:
		// Logger
		LoggerRef					log;	
		string						mixFileName;
		string						mError;
		//! name of the loaded shader file
		string						mFragFileName;
		//! include shader lines for header of loaded files
		std::string					header;
		// current frag string
		string						currentFrag;
		//! vector of fragment shaders
		vector<gl::GlslProgRef>		mFragmentShaders;
		bool						validFrag;

		//! parameters
		ParameterBagRef				mParameterBag;
		//! mix shader
		gl::GlslProgRef mMixShader;
	};
}