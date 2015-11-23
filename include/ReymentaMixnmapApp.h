/*
Copyright (c) 2014, Paul Houx - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and
the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS


#include "cinder/app/App.h"
#include "cinder/app/Platform.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Environment.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"
#include "cinder/Unicode.h"
#include "cinder/Utilities.h"

// UserInterface
#include "CinderImGui.h"
// parameters
#include "ParameterBag.h"
// spout
#include "SpoutWrapper.h"
// Utils
#include "Batchass.h"
// Console
#include "AppConsole.h"

#include <time.h>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Reymenta;

class ReymentaMixnmapApp : public App {
public:
	static void prepare(Settings* settings);

	void setup() override;
	void cleanup() override;

	void update() override;
	void draw() override;

	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void mouseMove(MouseEvent event) override;

	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;

	void resize() override;
	void fileDrop(FileDropEvent event) override;

	//void random();
private:
	void saveThumb();
	//! Sets the ShaderToy uniform variables.
	void setUniforms();
	//! Shader that will perform the transition to the next shader.
	gl::GlslProgRef mShaderTransition;
	//! Buffer containing the rendered output of the currently active shader.
	gl::FboRef      mBufferCurrent;
	//! Buffer containing the rendered output of the shader we are transitioning to.
	gl::FboRef      mBufferNext;
	//! Texture slots for our shader, based on ShaderToy.
	gl::TextureRef  mChannel0;
	gl::TextureRef  mChannel1;
	gl::TextureRef  mChannel2;
	gl::TextureRef  mChannel3;
	//! Our mouse position: xy = current position while mouse down, zw = last click position.
	vec4            mMouse;

	// parameters
	ParameterBagRef				mParameterBag;
	// utils
	BatchassRef					mBatchass;
	// console
	AppConsoleRef				mConsole;
	// spout
	SpoutWrapperRef				mSpout;
	// timeline
	Anim<float>					mTimer;

	gl::VboMeshRef				mMesh;
	gl::GlslProgRef				mProg;

	static const int			MODE_WARP = 1;

	bool						mUseBeginEnd;

	fs::path					mSettings;

	gl::TextureRef				mImage;
	WarpList					mWarps;

	Area						mSrcArea;

	// imgui
	float						color[4];
	float						backcolor[4];
	int							playheadPositions[12];
	float						speeds[12];
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
	//bool						sNewFrame;
	float						f = 0.0f;
	char						buf[64];
	bool						showConsole, showGlobal, showTextures, showTest, showMidi, showFbos, showTheme, showAudio, showShaders, showOSC, showChannels;
	bool						mouseGlobal;
	void						ShowAppConsole(bool* opened);
	// log only the first time
	bool						mFirstLaunch;
};
