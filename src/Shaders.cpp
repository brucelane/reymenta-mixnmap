/**
* \file Shaders.cpp
* \author Bruce LANE
* \date 20 november 2014
*
* Manages the shaders.
*
*/
#include "Shaders.h"

using namespace Reymenta;

Shaders::Shaders(ParameterBagRef aParameterBag)
{
	mParameterBag = aParameterBag;
	//! instanciate the logger class
	log = Logger::create("ShadersLog.txt");
	log->logTimedString("Shaders constructor");
	header = loadString(loadAsset("shaders/shadertoy.inc"));
	defaultVertexShader = loadString(loadAsset("shaders/default.vert"));
	defaultFragmentShader = loadString(loadAsset("shaders/default.glsl"));
	validFrag = false;

	string fileName;
	fs::path localFile;
	//! load mix shader
	fileName = "mix.glsl";
	localFile = getAssetPath("") / "shaders" / fileName;
	loadPixelFragmentShader(localFile.string());
	//! init some shaders
	for (size_t m = 0; m < 3; m++)
	{
		fileName = toString(m) + ".glsl";
		localFile = getAssetPath("") / "shaders" / fileName;
		loadPixelFragmentShader(localFile.string());
	}
}

void Shaders::resize()
{
	// change iResolution
	for (auto &shader : mFragmentShaders)
	{
		auto map = shader.prog->getActiveUniformTypes();
		if (map.find("iResolution") != map.end())
		{
			shader.prog->uniform("iResolution", vec3(mParameterBag->mFboWidth, mParameterBag->mFboHeight, 0.0f));
		}
	}
	auto mixMap = mMixShader->getActiveUniformTypes();
	if (mixMap.find("iResolution") != mixMap.end())
	{
		mMixShader->uniform("iResolution", vec3(mParameterBag->mFboWidth, mParameterBag->mFboHeight, 0.0f));
	}

}

gl::GlslProgRef Shaders::getShader(int aIndex)
{
	if (aIndex > mFragmentShaders.size() - 1) aIndex = mFragmentShaders.size() - 1;
	if (aIndex < 0) aIndex = 0;
	return mFragmentShaders[aIndex].prog;
};

bool Shaders::loadPixelFragmentShader(const fs::path &fragment_path)
{
	bool rtn = false;
	// reset 
	mParameterBag->iFade = false;
	mParameterBag->controlValues[13] = 1.0f;
	string mFile = fragment_path.string();
	try
	{
		if (mFile.find_last_of("\\") != std::string::npos) mFragFileName = mFile.substr(mFile.find_last_of("\\") + 1);
		if (fs::exists(fragment_path))
		{
			//validFrag = false;
			std::string fs = header + loadString(loadFile(fragment_path));
			rtn = setGLSLString(fs, mFragFileName);
		}
		else
		{
			log->logTimedString(mFragFileName + " loaded and compiled, does not exist:" + mFile);
		}
	}
	catch (gl::GlslProgCompileExc &exc)
	{
		mError = string(exc.what());
		log->logTimedString(mFile + " unable to load/compile shader:" + string(exc.what()));
	}
	catch (ci::Exception &e)
	{
		mError = string(e.what());
		log->logTimedString(mFile + " unable to load shader:" + string(e.what()));
	}
	if (!rtn)
	{
		// error load default fragment shader
		std::string fs = header + defaultFragmentShader;
		rtn = setGLSLString(fs, "default.glsl");
	}
	return rtn;
}
void Shaders::update()
{
	// get the current time with second-level accuracy
	auto now = boost::posix_time::second_clock::local_time();
	auto date = now.date();
	auto time = now.time_of_day();
	// set each uniform if it exists in the shader program
	// when compiled, only uniforms that are used remain in the program
	for (auto &shader : mFragmentShaders)
	{
		auto map = shader.prog->getActiveUniformTypes();
		if (map.find("iGlobalTime") != map.end())
		{
			shader.prog->uniform("iGlobalTime", static_cast<float>(getElapsedSeconds()));
		}
		if (map.find("iDate") != map.end())
		{
			shader.prog->uniform("iDate", vec4(date.year(), date.month(), date.day_number(), time.total_seconds()));
		}
		if (map.find("iMouse") != map.end())
		{
			shader.prog->uniform("iMouse", mParameterBag->iMouse);
		}
		if (map.find("iColor") != map.end())
		{
			shader.prog->uniform("iColor", vec3(mParameterBag->controlValues[1], mParameterBag->controlValues[2], mParameterBag->controlValues[3]));
		}
		if (map.find("iBackgroundColor") != map.end())
		{
			shader.prog->uniform("iBackgroundColor", vec3(mParameterBag->controlValues[5], mParameterBag->controlValues[6], mParameterBag->controlValues[7]));
		}
		if (map.find("iChannel0") != map.end())
		{
			shader.prog->uniform("iChannel0", 0);
		}
		if (map.find("iChannel1") != map.end())
		{
			shader.prog->uniform("iChannel1", 1);
		}
	}
	auto mixMap = mMixShader->getActiveUniformTypes();
	if (mixMap.find("iGlobalTime") != mixMap.end())
	{
		mMixShader->uniform("iGlobalTime", static_cast<float>(getElapsedSeconds()));
	}
	if (mixMap.find("iCrossfade") != mixMap.end())
	{
		//mMixShader->uniform("iCrossfade", mParameterBag->controlValues[15]);//TODO a crossfader for each warp
	}
	if (mixMap.find("iAlpha") != mixMap.end())
	{
		mMixShader->uniform("iAlpha", mParameterBag->controlValues[4]);
	}
	if (mixMap.find("iChannel0") != mixMap.end())
	{
		mMixShader->uniform("iChannel0", 0);
	}
	if (mixMap.find("iChannel1") != mixMap.end())
	{
		mMixShader->uniform("iChannel1", 1);
	}
}
bool Shaders::setGLSLString(string pixelFrag, string fileName)
{
	currentFrag = pixelFrag;
	Shada newShada;
	newShada.name = fileName;
	newShada.active = true;
	newShada.prog = gl::GlslProg::create(gl::GlslProg::Format().vertex(defaultVertexShader.c_str()).fragment(currentFrag.c_str()));
	try
	{
		// special treatment for mix.glsl
		if (fileName == "mix.glsl")
		{
			mMixShader = gl::GlslProg::create(gl::GlslProg::Format().vertex(defaultVertexShader.c_str()).fragment(currentFrag.c_str()));
			log->logTimedString("setGLSLString success for mMixShader ");
			auto mixMap = mMixShader->getActiveUniformTypes();
			for (const auto &pair : mixMap)
			{
				log->logTimedString(pair.first);
			}
			if (mixMap.find("iResolution") != mixMap.end())
			{
				mMixShader->uniform("iResolution", vec3(getWindowWidth(), getWindowHeight(), 0.0f));
			}
		}
		else
		{
			// searching first index of not running shader
			if (mFragmentShaders.size() < mParameterBag->MAX)
			{
				mFragmentShaders.push_back(newShada);
			}
			else
			{
				// load the new shader
				mFragmentShaders[mFragmentShaders.size() - 1] = newShada;
			}
			//preview the new loaded shader
			mParameterBag->mCurrentShadaFboIndex = mFragmentShaders.size() - 1;
			log->logTimedString("setGLSLString success, mFragmentShaders size " + static_cast<ostringstream*>(&(ostringstream() << mFragmentShaders.size() - 1))->str());
			// check that uniforms exist before setting the constant uniforms
			auto map = mFragmentShaders[mFragmentShaders.size() - 1].prog->getActiveUniformTypes();

			log->logTimedString("Found uniforms for " + fileName);
			for (const auto &pair : map)
			{
				log->logTimedString(pair.first);
			}
			console() << endl;
			if (map.find("iResolution") != map.end())
			{
				mFragmentShaders[mFragmentShaders.size() - 1].prog->uniform("iResolution", vec3(getWindowWidth(), getWindowHeight(), 0.0f));
			}

		}
		mError = "";
		validFrag = true;
	}
	catch (gl::GlslProgCompileExc &exc)
	{
		validFrag = false;
		mError = string(exc.what());
		log->logTimedString("setGLSLString error: " + mError);
	}
	return validFrag;
}

Shaders::~Shaders()
{
	log->logTimedString("Shaders destructor");
}
string Shaders::getFragError()
{
	return mError;
}
