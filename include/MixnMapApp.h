/*
Copyright (c) 2014, Bruce Lane - Martin Blasko All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

This file is part of Cinder-MIDI.

Cinder-MIDI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Cinder-MIDI is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Cinder-MIDI.  If not, see <http://www.gnu.org/licenses/>.
*/

// don't forget to add winmm.lib to the linker

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
//#include "OSCSender.h"

#include <list>

// UserInterface
#include "CinderImGui.h"
// parameters
#include "ParameterBag.h"
// json
#include "JSONWrapper.h"
// audio
#include "AudioWrapper.h"
// spout
#include "SpoutWrapper.h"
// Utils
#include "Batchass.h"
// Console
#include "AppConsole.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Reymenta;

#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))


class MixNMapApp : public App {
public:
	void 						setup();
	void 						update();
	void						draw();
	void 						keyDown(KeyEvent event);
	void 						keyUp(KeyEvent event);
	void 						fileDrop(FileDropEvent event);
	void 						shutdown();
	void 						resize();
	void 						mouseMove(MouseEvent event);
	void 						mouseDown(MouseEvent event);
	void 						mouseDrag(MouseEvent event);
	void 						mouseUp(MouseEvent event);
	void 						mouseWheel(MouseEvent event);
	void 						saveThumb();
	//! Override to receive window activate events
	void						activate();
	//! Override to receive window deactivate events
	void						deactivate();

private:
	// parameters
	ParameterBagRef				mParameterBag;
	// json
	JSONWrapperRef				mJson;
	// audio
	AudioWrapperRef				mAudio;
	// spout
	SpoutWrapperRef				mSpout;
	// utils
	BatchassRef					mBatchass;
	// console
	AppConsoleRef				mConsole;
	// timeline
	Anim<float>					mTimer;

	// misc
	int							mSeconds;
	// windows
	WindowRef					mMainWindow;
	// render
	bool						removeUI;

	static const int			MODE_WARP = 1;

	// imgui
	float						color[4];
	float						backcolor[4];
	int							playheadPositions[12];
	float						speeds[12];
	// mPreviewFboWidth 80 mPreviewFboHeight 60 margin 10 inBetween 15
	int							w;
	int							h;
	int							displayHeight;
	int							xPos;
	int							yPos;
	int							largeW;
	int							largeH;
	int							largePreviewW;
	int							largePreviewH;
	int							margin;
	int							inBetween;

	float						f = 0.0f;
	char						buf[64];

	bool						showConsole, showGlobal, showTextures, showTest, showMidi, showFbos, showTheme, showAudio, showShaders, showOSC, showChannels;
	bool						mouseGlobal;
	void						ShowAppConsole(bool* opened);

};