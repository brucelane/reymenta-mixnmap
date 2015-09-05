#pragma once
#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "cinder/Utilities.h"

// parameters
#include "ParameterBag.h"
// Utils
#include "Batchass.h"
// WebSockets
#include "WebSocketsWrapper.h"

// UserInterface
#include "CinderImGui.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))

namespace Reymenta
{
	// stores the pointer to the AppConsole instance
	typedef std::shared_ptr<class AppConsole> AppConsoleRef;

	class AppConsole
	{
	public:
		AppConsole(ParameterBagRef aParameterBag, BatchassRef aBatchass, WebSocketsRef aWebSockets);
		static AppConsoleRef	create(ParameterBagRef aParameterBag, BatchassRef aBatchass, WebSocketsRef aWebSockets)
		{
			return shared_ptr<AppConsole>(new AppConsole(aParameterBag, aBatchass, aWebSockets));
		}
		void    ClearLog();
		void    AddLog(const char* fmt, ...);
		void    Run(const char* title, bool* opened);
		void    ExecCommand(const char* command_line);

	private:
		// parameters
		ParameterBagRef				mParameterBag;
		// utils
		BatchassRef					mBatchass;
		// WebSockets
		WebSocketsRef				mWebSockets;

		char                  InputBuf[256];
		ImVector<char*>       Items;
		bool                  ScrollToBottom;
		ImVector<char*>       History;
		int                   HistoryPos;    // -1: new line, 0..History.size()-1 browsing history.
		ImVector<const char*> Commands;	
		static int AppConsole::TextEditCallbackStub(ImGuiTextEditCallbackData* data)
		{
			AppConsole* console = (AppConsole*)data->UserData;
			return console->TextEditCallback(data);
		}
		int     TextEditCallback(ImGuiTextEditCallbackData* data);

	};
}