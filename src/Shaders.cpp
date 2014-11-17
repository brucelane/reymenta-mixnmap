#include "Shaders.h"

using namespace Reymenta;

Shaders::Shaders(ParameterBagRef aParameterBag)
{
	mParameterBag = aParameterBag;
	// instanciate the logger class
	log = Logger::create("ReymentaRenderShadersLog.txt");
	log->logTimedString("Shaders constructor");

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
	header = loadString(loadAsset("shaders/shadertoy.inc"));
	validFrag = false;
	string fileName;
	fs::path localFile;
	for (size_t m = 0; m < 8; m++)
	{
		fileName = toString(m) + ".glsl";
		localFile = getAssetPath("") / "shaders" / fileName;
		loadPixelFragmentShader(localFile.string());
	}
	// init with passthru shader if something goes wrong	
	for (size_t m = mFragmentShaders.size(); m < 8; m++)
	{
		mFragmentShaders.push_back(gl::GlslProg::create(loadResource(PASSTHROUGH2_VERT), loadResource(PASSTHROUGH_FRAG)));
	}

}
bool Shaders::loadPixelFragmentShader(string aFilePath)
{
	bool rtn = false;
	// reset 
	mParameterBag->iFade = false;
	mParameterBag->controlValues[13] = 1.0f;
	try
	{
		fs::path fr = aFilePath;
		string name = "unknown";
		string mFile = fr.string();
		if (mFile.find_last_of("\\") != std::string::npos) name = mFile.substr(mFile.find_last_of("\\") + 1);
		mFragFileName = name;
		if (fs::exists(fr))
		{
			//validFrag = false;
			std::string fs = header + loadString(loadFile(aFilePath));

			rtn = setGLSLString(fs);
		}
		else
		{
			log->logTimedString(mFragFileName + " loaded and compiled, does not exist:" + aFilePath);
		}
	}
	catch (gl::GlslProgCompileExc &exc)
	{
		mError = string(exc.what());
		log->logTimedString(aFilePath + " unable to load/compile shader:" + string(exc.what()));
	}
	catch (const std::exception &e)
	{
		mError = string(e.what());
		log->logTimedString(aFilePath + " unable to load shader:" + string(e.what()));
	}

	return rtn;
}
bool Shaders::setGLSLString(string pixelFrag)
{
	currentFrag = pixelFrag;
	try
	{
		// searching first index of not running shader
		if (mFragmentShaders.size() < 8)
		{
			mFragmentShaders.push_back(gl::GlslProg::create(NULL, currentFrag.c_str()));
		}
		else
		{
			bool indexFound = false;
			int foundIndex = -1;
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
			mFragmentShaders[foundIndex] = gl::GlslProg::create(NULL, currentFrag.c_str());
			//preview the new loaded shader
			mParameterBag->mPreviewFragIndex = foundIndex;
		}
		log->logTimedString("setGLSLString success");
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
/*string Shaders::getFragStringFromFile(string fileName)
{
	string rtn = "";
	try
	{
		rtn = loadString(loadAsset("shaders/templates/" + fileName));
	}
	catch (const std::exception &e)
	{
		log->logTimedString(fileName + " unable to load string from file:" + string(e.what()));
	}
	return rtn;
}



string Shaders::getFileName(string aFilePath)
{
	string fName;
	if (aFilePath.find_last_of("\\") != std::string::npos)
	{
		fName = aFilePath.substr(aFilePath.find_last_of("\\") + 1);
	}
	else
	{
		fName = aFilePath;
	}
	return fName;
}
string Shaders::getNewFragFileName(string aFilePath)
{
	string fName;
	if (aFilePath.find_last_of("\\") != std::string::npos)
	{
		fName = aFilePath.substr(aFilePath.find_last_of("\\") + 1);
	}
	else
	{
		fName = aFilePath;
	}
	return fName + ".frag";
}*/
