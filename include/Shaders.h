#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Utilities.h"
#include "cinder/Timeline.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
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
		void update();
		void resize();
		string getFragError();
		gl::GlslProgRef getShader(int aIndex) { return mFragmentShaders[aIndex]; };
		gl::GlslProgRef getMixShader() { return mMixShader; };
		bool loadPixelFragmentShader(const fs::path &fragment_path);
		string getFragFileName() { return mFragFileName; };
		bool setGLSLString(string pixelFrag);

	private:
		// Logger
		LoggerRef					log;	
		string						mixFileName;
		string						mError;
		//! name of the loaded shader file
		string						mFragFileName;
		//! include shader lines for header of loaded files
		std::string					header;
		//! vertex shader string
		std::string					defaultVertexShader;
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