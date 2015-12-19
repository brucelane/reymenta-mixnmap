#include "AppConsole.h"

using namespace Reymenta;

AppConsole::AppConsole(ParameterBagRef aParameterBag, BatchassRef aBatchass)
{
	mParameterBag = aParameterBag;
	mBatchass = aBatchass;
	ClearLog();
	HistoryPos = -1;
	Commands.push_back("HELP");
	Commands.push_back("HISTORY");
	Commands.push_back("CLEAR");
	Commands.push_back("DEBUG");
	Commands.push_back("WARPCREATE");
	Commands.push_back("WARPF");
	Commands.push_back("MODEMIX");
	Commands.push_back("MODEAUDIO");
	Commands.push_back("MODEWARP");
	Commands.push_back("MODESPHERE");
	Commands.push_back("MODEMESH");
	Commands.push_back("WSCNX");
	Commands.push_back("WSPING");
	Commands.push_back("WSCNF");
	Commands.push_back("LOADSTOP");
	Commands.push_back("TEMPO");
}
void AppConsole::ClearLog()
{
	for (size_t i = 0; i < Items.size(); i++)
		ImGui::MemFree(Items[i]);
	Items.clear();
	ScrollToBottom = true;
}
void AppConsole::AddLog(const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	ImFormatStringV(buf, IM_ARRAYSIZE(buf), fmt, args);
	va_end(args);
	Items.push_back(ImStrdup(buf));
	ScrollToBottom = true;
}

void AppConsole::Run(const char* title, bool* opened)
{
	//ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin(title, opened))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("TAB=text completion");

	// TODO: display from bottom
	// TODO: clip manually

	if (ImGui::SmallButton("Help"))
	{
		AddLog("Commands:");
		for (size_t i = 0; i < Commands.size(); i++)
			AddLog("- %s", Commands[i]);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Clear")) ClearLog();

	ImGui::Separator();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	static ImGuiTextFilter filter;
	filter.Draw("Filter (\"incl,-excl\") (\"error\")", 70);
	//if (ImGui::IsItemHovered()) ImGui::SetKeyboardFocusHere(-1); // Auto focus on hover
	ImGui::PopStyleVar();
	ImGui::Separator();

	// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
	// NB- if you have thousands of entries this approach may be too inefficient. You can seek and display only the lines that are visible - CalcListClipping() is a helper to compute this information.
	// If your items are of variable size you may want to implement code similar to what CalcListClipping() does. Or split your data into fixed height items to allow random-seeking into your list.
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	for (size_t i = 0; i < Items.size(); i++)
	{
		const char* item = Items[i];
		if (!filter.PassFilter(item))
			continue;
		ImVec4 col(1, 1, 1, 1); // A better implement may store a type per-item. For the sample let's just parse the text.
		if (strstr(item, "[error]")) col = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
		else if (strncmp(item, "# ", 2) == 0) col = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item);
		ImGui::PopStyleColor();
	}
	if (ScrollToBottom)
		ImGui::SetScrollPosHere();
	ScrollToBottom = false;
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	// Command-line
	if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
	{
		char* input_end = InputBuf + strlen(InputBuf);
		while (input_end > InputBuf && input_end[-1] == ' ') input_end--; *input_end = 0;
		if (InputBuf[0])
			ExecCommand(InputBuf);
		strcpy(InputBuf, "");
	}

	// Demonstrate keeping auto focus on the input box
	if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1); // Auto focus

	ImGui::End();
}
void AppConsole::ExecCommand(const char* command_line)
{
	AddLog("# %s\n", command_line);

	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for (int i = (int)History.size() - 1; i >= 0; i--)
		if (ImStricmp(History[i], command_line) == 0)
		{
			ImGui::MemFree(History[i]);
			History.erase(History.begin() + i);
			break;
		}
	History.push_back(ImStrdup(command_line));

	// Process command
	if (ImStricmp(command_line, "CLEAR") == 0)
	{
		ClearLog();
	}
	else if (ImStricmp(command_line, "HELP") == 0)
	{
		AddLog("Commands:");
		for (size_t i = 0; i < Commands.size(); i++)
			AddLog("- %s", Commands[i]);
	}
	else if (ImStricmp(command_line, "HISTORY") == 0)
	{
		for (size_t i = History.size() >= 10 ? History.size() - 10 : 0; i < History.size(); i++)
			AddLog("%3d: %s\n", i, History[i]);
	}
	else if (ImStricmp(command_line, "DEBUG") == 0)
	{
		mParameterBag->iDebug = !mParameterBag->iDebug;
	}
	else if (ImStricmp(command_line, "WARPCREATE") == 0)
	{
		mBatchass->createWarp();
	}
	else if (ImStrnicmp(command_line, "WARPF", 5) == 0)
	{
		// for instance WARPF 0 4
		if (strlen(command_line) > 8)
		{
			int index = command_line[6];
			int fbo = command_line[8];

			mBatchass->assignFboToWarp(index - 48, fbo - 48);
		}
	}
	else if (ImStricmp(command_line, "MODEMIX") == 0)
	{
		mBatchass->changeMode(mParameterBag->MODE_MIX);
	}
	else if (ImStricmp(command_line, "MODEAUDIO") == 0)
	{
		mBatchass->changeMode(mParameterBag->MODE_AUDIO);
	}
	else if (ImStricmp(command_line, "MODEWARP") == 0)
	{
		mBatchass->changeMode(mParameterBag->MODE_WARP);
	}
	else if (ImStricmp(command_line, "MODESPHERE") == 0)
	{
		mBatchass->changeMode(mParameterBag->MODE_SPHERE);
	}
	else if (ImStricmp(command_line, "MODEMESH") == 0)
	{
		mBatchass->changeMode(mParameterBag->MODE_MESH);
	}
	else if (ImStricmp(command_line, "LOADSTOP") == 0)
	{
		mBatchass->stopLoading();
	}
	else if (ImStrnicmp(command_line, "TEMPO", 5) == 0)
	{
		if (strlen(command_line) > 7)
		{
			string to = command_line;
			int firstDigit = to.find_first_of("0123456789");			
			if (firstDigit > -1) mParameterBag->mTempo = std::stoi(to.substr(firstDigit));
		}		
	}
	else if (ImStricmp(command_line, "WSCNX") == 0)
	{
		mBatchass->wsConnect();
	}
	else if (ImStricmp(command_line, "WSPING") == 0)
	{
		mBatchass->wsPing();
	}
	else if (ImStricmp(command_line, "WSCNF") == 0)
	{
		if (mParameterBag->mIsWebSocketsServer)
		{
			AddLog("Server %s\n", mParameterBag->mWebSocketsHost.c_str());
		}
		else
		{
			//AddLog("Client %s %s\n", mParameterBag->mWebSocketsHost.c_str(), (mWebSockets->isClientConnected() ? "true" : "false"));
			AddLog("Client %s \n", mParameterBag->mWebSocketsHost.c_str());
		}
	}
	else
	{
		AddLog("Unknown command: '%s'\n", command_line);
	}
}

int AppConsole::TextEditCallback(ImGuiTextEditCallbackData* data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Example of TEXT COMPLETION

		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (ImCharIsSpace(c) || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build a list of candidates
		ImVector<const char*> candidates;
		for (size_t i = 0; i < Commands.size(); i++)
			if (ImStrnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(Commands[i]);

		if (candidates.size() == 0)
		{
			// No match
			AddLog("No match for \"%.*s\"!\n", word_end - word_start, word_start);
		}
		else if (candidates.size() == 1)
		{
			// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (size_t i = 0; i < candidates.size() && all_candidates_matches; i++)
					if (i == 0)
						c = toupper(candidates[i][match_len]);
					else if (c != toupper(candidates[i][match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
			}

			// List matches
			AddLog("Possible matches:\n");
			for (size_t i = 0; i < candidates.size(); i++)
				AddLog("- %s\n", candidates[i]);
		}

		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = (int)(History.size() - 1);
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= (int)History.size())
					HistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != HistoryPos)
		{
			ImFormatString(data->Buf, data->BufSize, "%s", (HistoryPos >= 0) ? History[HistoryPos] : "");
			data->BufDirty = true;
			data->CursorPos = data->SelectionStart = data->SelectionEnd = (int)strlen(data->Buf);
		}
	}
	}
	return 0;
}
// helpers copied from imgui.cpp
int AppConsole::ImStricmp(const char* str1, const char* str2)
{
	int d;
	while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; }
	return d;
}

int AppConsole::ImStrnicmp(const char* str1, const char* str2, int count)
{
	int d = 0;
	while (count > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; count--; }
	return d;
}

char* AppConsole::ImStrdup(const char *str)
{
	char *buff = (char*)ImGui::MemAlloc(strlen(str) + 1);
	IM_ASSERT(buff);
	strcpy(buff, str);
	return buff;
}

int AppConsole::ImStrlenW(const ImWchar* str)
{
	int n = 0;
	while (*str++) n++;
	return n;
}

//const AppConsole::ImWchar* ImStrbolW(const ImWchar* buf_mid_line, const ImWchar* buf_begin) // find beginning-of-line
//{
//	while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
//		buf_mid_line--;
//	return buf_mid_line;
//}

const char* AppConsole::ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
{
	if (!needle_end)
		needle_end = needle + strlen(needle);

	const char un0 = (char)toupper(*needle);
	while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
	{
		if (toupper(*haystack) == un0)
		{
			const char* b = needle + 1;
			for (const char* a = haystack + 1; b < needle_end; a++, b++)
				if (toupper(*a) != toupper(*b))
					break;
			if (b == needle_end)
				return haystack;
		}
		haystack++;
	}
	return NULL;
}

int AppConsole::ImFormatString(char* buf, int buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int w = vsnprintf(buf, buf_size, fmt, args);
	va_end(args);
	buf[buf_size - 1] = 0;
	return (w == -1) ? buf_size : w;
}

int AppConsole::ImFormatStringV(char* buf, int buf_size, const char* fmt, va_list args)
{
	int w = vsnprintf(buf, buf_size, fmt, args);
	buf[buf_size - 1] = 0;
	return (w == -1) ? buf_size : w;
}
