#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "cinder/DataSource.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta
{
	// stores the pointer to the Logger instance
	typedef std::shared_ptr<class Logger> LoggerRef;

	class Logger {
	public:
		Logger(string aLogFile);
		virtual				~Logger();

		void				logString(std::string msg);
		void				logTimedString(std::string msg);
		static LoggerRef	create(string aLogFile)
		{
			return shared_ptr<Logger>(new Logger(aLogFile));
		}

	private:
		std::string			logFileName;
		std::ofstream		logFile;

	};


}