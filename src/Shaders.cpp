#include "Shaders.h"

using namespace Reymenta;

Shaders::Shaders(ParameterBagRef aParameterBag)
{
	mParameterBag = aParameterBag;
	// instanciate the logger class
	log = Logger::create("ReymentaRenderShadersLog.txt");
	log->logTimedString("Shaders constructor");
	header = loadString(loadAsset("shaders/shadertoy.inc"));
	defaultVertexShader = loadString(loadAsset("shaders/default.vert"));
	validFrag = false;

	//load mix shader
	try
	{
		fs::path mixFragFile = getAssetPath("") / "shaders" / "mix.frag";
		if (fs::exists(mixFragFile))
		{
			mMixShader = gl::GlslProg::create(loadResource(PASSTHROUGH2_VERT), loadFile(mixFragFile));
		}
		else
		{
			log->logTimedString("mix.frag does not exist:");
		}
	}
	catch (gl::GlslProgCompileExc &exc)
	{
		mError = string(exc.what());
		log->logTimedString("unable to load/compile shader:" + string(exc.what()));
	}
	catch (const std::exception &e)
	{
		mError = string(e.what());
		log->logTimedString("unable to load shader:" + string(e.what()));
	}
	string fileName;
	fs::path localFile;
	for (size_t m = 0; m < 3; m++)
	{
		fileName = toString(m) + ".glsl";
		localFile = getAssetPath("") / "shaders" / fileName;
		loadPixelFragmentShader(localFile.string());
	}
	// init with passthru shader if something goes wrong	
	/*for (size_t m = mFragmentShaders.size(); m < 8; m++)
	{
		mFragmentShaders.push_back(gl::GlslProg::create(loadResource(PASSTHROUGH2_VERT), loadResource(PASSTHROUGH_FRAG)));
	}*/
}
void Shaders::resize()
{
	for (auto &shader : mFragmentShaders)
	{
		auto map = shader->getActiveUniformTypes();
		if (map.find("iResolution") != map.end())
		{
			shader->uniform("iResolution", vec3(getWindowWidth(), getWindowHeight(), 0.0f));
		}
	}
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
		auto map = shader->getActiveUniformTypes();
		if (map.find("iGlobalTime") != map.end())
		{
			shader->uniform("iGlobalTime", static_cast<float>(getElapsedSeconds()));
		}
		if (map.find("iDate") != map.end())
		{
			shader->uniform("iDate", vec4(date.year(), date.month(), date.day_number(), time.total_seconds()));
		}
		if (map.find("iMouse") != map.end())
		{
			shader->uniform("iMouse", mParameterBag->iMouse);
		}
	}
}
gl::GlslProgRef Shaders::getShader(int aIndex) 
{ 
	if (aIndex > mFragmentShaders.size() - 1) aIndex = mFragmentShaders.size() - 1;
	if (aIndex < 0) aIndex = 0;
	return mFragmentShaders[aIndex];
};

bool Shaders::loadPixelFragmentShader(const fs::path &fragment_path)
{
	bool rtn = false;
	// reset 
	mParameterBag->iFade = false;
	mParameterBag->controlValues[13] = 1.0f;
	string name = "unknown";
	string mFile = fragment_path.string();
	try
	{
		if (mFile.find_last_of("\\") != std::string::npos) name = mFile.substr(mFile.find_last_of("\\") + 1);
		mFragFileName = name;
		if (fs::exists(fragment_path))
		{
			//validFrag = false;
			std::string fs = header + loadString(loadFile(fragment_path));
			rtn = setGLSLString(fs);
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

	return rtn;
}
bool Shaders::setGLSLString(string pixelFrag)
{
	currentFrag = pixelFrag;
	int foundIndex = -1;
	try
	{
		// searching first index of not running shader
		if (mFragmentShaders.size() < 8)
		{
			mFragmentShaders.push_back(gl::GlslProg::create(gl::GlslProg::Format().vertex(defaultVertexShader.c_str()).fragment(currentFrag.c_str())));
			foundIndex = mFragmentShaders.size() - 1;
		}
		else
		{
			bool indexFound = false;
			if (mParameterBag->mDirectRender)
			{
				foundIndex = mParameterBag->mRightFragIndex;
			}
			else
			{
				while (!indexFound)
				{
					foundIndex++;
					if (foundIndex != mParameterBag->mLeftFragIndex && foundIndex != mParameterBag->mRightFragIndex && foundIndex != mParameterBag->mPreviewFragIndex) indexFound = true;
					if (foundIndex > mFragmentShaders.size() - 1) indexFound = true;
				}
			}
			// load the new shader
			mFragmentShaders[foundIndex] = gl::GlslProg::create(gl::GlslProg::Format().vertex(defaultVertexShader.c_str()).fragment(currentFrag.c_str()));

			//preview the new loaded shader
			mParameterBag->mPreviewFragIndex = foundIndex;
		}
		log->logTimedString("setGLSLString success");
		// check that uniforms exist before setting the constant uniforms
		auto map = mFragmentShaders[foundIndex]->getActiveUniformTypes();

		log->logTimedString("Found uniforms:");
		for (const auto &pair : map)
		{
			log->logTimedString(pair.first );
		}
		console() << endl;
		if (map.find("iResolution") != map.end())
		{
			mFragmentShaders[foundIndex]->uniform("iResolution", vec3(getWindowWidth(), getWindowHeight(), 0.0f));
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
