/*
Copyright (c) 2010-2014, Paul Houx - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

This file is part of Cinder-Warping.

Cinder-Warping is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Cinder-Warping is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Cinder-Warping.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Timeline.h"
// test for multitouch
#include "cinder/System.h"
// warps
#include "WarpBilinear.h"
#include "WarpPerspective.h"
#include "WarpPerspectiveBilinear.h"

// window manager
#include "WindowMngr.h"
// UserInterface
#include "UI.h"
// parameters
#include "ParameterBag.h"
// textures
#include "Textures.h"
// logger
#include "Logger.h"
// spout
#include "SpoutWrapper.h"
// OSC
#include "OSCWrapper.h"
// warps
//#include "WarpWrapper.h"
// audio
#include "AudioWrapper.h"

using namespace ci;
using namespace ci::app;
using namespace ph::warping;
using namespace std;
using namespace Reymenta;

class MixnMapApp : public AppNative {
public:
	void						prepareSettings(Settings *settings);
	void						setup();
	void						shutdown();
	void						update();
	void						resize();

	// mouse events
	void						mouseMove(MouseEvent event);
	void						mouseDown(MouseEvent event);
	void						mouseDrag(MouseEvent event);
	void						mouseUp(MouseEvent event);

	// keyboard events
	void						keyDown(KeyEvent event);
	void						keyUp(KeyEvent event);
	//! allow file drop of images, shaders
	void						fileDrop(FileDropEvent event);

	void						updateWindowTitle();
	void						setSelectedWarp(int index);

private:
	// windows
	WindowRef					mMainWindow;
	void						windowManagement();
	void						getWindowsResolution();
	void						drawMain();
	WindowRef					mCodeEditorWindow;
	bool						mCursorVisible;
	bool						mIsShutDown;
	// render
	void						createRenderWindow();
	void						deleteRenderWindows();
	vector<WindowMngr>			allRenderWindows;
	void						drawRender();
	void						showCodeCallback();
	void						createUIWindow();
	boost::signals2::connection	mainDrawConnection;
	boost::signals2::connection	renderDrawConnection;
	//! timeline to save thumb for shader
	Anim<float>					mTimer;
	void						saveThumb();

	// minimalUI
	UIRef						mUI;
	// parameters
	ParameterBagRef				mParameterBag;
	// Logger
	LoggerRef					log;
	// Shaders
	ShadersRef					mShaders;
	// Textures
	TexturesRef					mTextures;
	// spout
	SpoutWrapperRef				mSpout;
	// osc
	OSCRef						mOSC;
	// warps
	const string				warpsFileName = "MixnMapWarps.xml";
	WarpList					mWarps;

	// audio
	AudioWrapperRef				mAudio;

	bool						newLogMsg;
	string						mLogMsg;
};