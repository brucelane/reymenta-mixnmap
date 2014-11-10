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


}

Shaders::~Shaders()
{
	log->logTimedString("Shaders destructor");
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



string Shaders::getFragError()
{
	return mError;
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
