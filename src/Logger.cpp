#include "Logger.h"

using namespace Reymenta;

Logger::Logger(string aLogFile)
{

	fs::path fsPath = getAssetPath("") / "log" / aLogFile;
	if (!fs::exists(fsPath.parent_path()))
	{
		fs::create_directories(fsPath.parent_path());
	}
	logFileName = fsPath.string();
	if (logFile.is_open())
	{
		logFile.close();
	}

	// replace: std::ios::out
	logFile.open(logFileName.c_str(), std::ios::out);// Append: std::ios::app);
}

Logger::~Logger()
{
	logTimedString("quit");
	// Close file
	if (logFile.is_open())
	{
		logFile.close();
	}
}

void Logger::logString(std::string msg)
{
	logFile << msg.c_str() << "\n";
	logFile.flush();
	//
#ifdef _DEBUG
	// debug mode
	std::cout << msg << std::endl;
#endif  // _DEBUG
}

void Logger::logTimedString(std::string msg)
{
	time_t currentTime = time(0);
	struct tm * now = localtime(&currentTime);

	logFile << "[" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "] " << msg << "\n";
	logFile.flush();
#ifdef _DEBUG
	// debug mode
	std::cout << msg << std::endl;
#endif  // _DEBUG
	console() << msg << std::endl;
}

