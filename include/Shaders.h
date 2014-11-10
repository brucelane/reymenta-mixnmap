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
		gl::GlslProgRef getMixShader() { return mMixShader; };

		/*void loadFragmentShader(boost::filesystem::path aFilePath);
		string getFileName(string aFilePath);
		string getNewFragFileName( string aFilePath);
		string getFragStringFromFile( string fileName );*/
	private:
		// Logger
		LoggerRef					log;	
		string						mixFileName;
		string						mError;
		// parameters
		ParameterBagRef				mParameterBag;
		// mix shader
		gl::GlslProgRef mMixShader;
	};
}