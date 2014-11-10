#pragma once

#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// WindowMngr class
class WindowMngr
{
public:
	WindowMngr( string name, int width, int height, WindowRef wRef )
		: mName( name ), mWidth ( width ), mHeight ( height ), mWRef ( wRef )
	{
		//mWindows.push_back( mWRef );
	}

	WindowRef mWRef;
	string mName;

private:
	int mWidth;
	int mHeight;
	//static std::vector<WindowRef> mWindows;
};
