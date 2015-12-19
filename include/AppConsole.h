#pragma once
#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"

// parameters
#include "ParameterBag.h"
// Utils
#include "Batchass.h"

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
		AppConsole(ParameterBagRef aParameterBag, BatchassRef aBatchass);
		static AppConsoleRef	create(ParameterBagRef aParameterBag, BatchassRef aBatchass)
		{
			return shared_ptr<AppConsole>(new AppConsole(aParameterBag, aBatchass));
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

		// helpers copied from imgui.cpp
		int ImStricmp(const char* str1, const char* str2);
		int ImStrnicmp(const char* str1, const char* str2, int count);
		char* ImStrdup(const char *str);
		int ImStrlenW(const ImWchar* str);
		//const ImWchar* ImStrbolW(const ImWchar* buf_mid_line, const ImWchar* buf_begin);
		const char* ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);
		int ImFormatString(char* buf, int buf_size, const char* fmt, ...);
		int ImFormatStringV(char* buf, int buf_size, const char* fmt, va_list args);
		static inline bool  ImCharIsSpace(int c)            { return c == ' ' || c == '\t' || c == 0x3000; }


	};
}
