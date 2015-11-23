/*
TODO
- warp select mix fbo texture
- flip horiz
- check flip H and V (spout also)
- sort fbo names and indexes (warps only 4 or 5 inputs)
- spout texture 10 create shader 10.glsl(ThemeFromBrazil) iChannel0
- warpwrapper handle texture mode 0 for spout (without fbo)
- put sliderInt instead of popups //warps next
- proper slitscan h and v //wip
- proper rotation
- badtv in mix.frag

*/

#include "ReymentaMixnmapApp.h"

void ReymentaMixnmapApp::prepare(Settings* settings)
{
	// Do not allow resizing our window. Feel free to remove this limitation.
	settings->setResizable(false);
	settings->setBorderless();
}

void ReymentaMixnmapApp::setup()
{
	int wr;
	// parameters
	mParameterBag = ParameterBag::create();
	mParameterBag->mLiveCode = true;
	mParameterBag->mRenderThumbs = false;
	// utils
	mBatchass = Batchass::create(mParameterBag);
	CI_LOG_V("reymenta setup");
	mFirstLaunch = true;
	wr = mBatchass->getWindowsResolution();
	setWindowSize(mParameterBag->mMainWindowWidth, mParameterBag->mMainWindowHeight);
	// setup shaders and textures
	mBatchass->setup();
	// Setting an unrealistically high frame rate effectively
	// disables frame rate limiting
	//settings->setFrameRate(10000.0f);
	setFrameRate(60.0f);
	//settings->setWindowPos(ivec2(w - mParameterBag->mMainWindowWidth, 0));
	setWindowPos(ivec2(0, 0));
	mParameterBag->iResolution.x = mParameterBag->mRenderWidth;
	mParameterBag->iResolution.y = mParameterBag->mRenderHeight;
	mParameterBag->mRenderResolution = ivec2(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	mParameterBag->mRenderResoXY = vec2(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	mParameterBag->mRenderPosXY = ivec2(mParameterBag->mRenderX, mParameterBag->mRenderY);//20141214 was 0
#if defined(DEBUG)
	setWindowSize(640, 480);
#else
	// if mStandalone, put on the 2nd screen
	if (mParameterBag->mStandalone)
	{
		setWindowSize(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
		setWindowPos(ivec2(mParameterBag->mRenderX, mParameterBag->mRenderY));
	}

#endif
	mParameterBag->mMode = MODE_WARP;

	// Load our textures and transition shader in the main thread.
	try {
		gl::Texture::Format fmt;
		fmt.setWrap(GL_REPEAT, GL_REPEAT);

		mChannel0 = gl::Texture::create(loadImage(loadAsset("presets/tex16.png")), fmt);
		mChannel1 = gl::Texture::create(loadImage(loadAsset("presets/tex06.jpg")), fmt);
		mChannel2 = gl::Texture::create(loadImage(loadAsset("presets/tex09.jpg")), fmt);
		mChannel3 = gl::Texture::create(loadImage(loadAsset("presets/tex02.jpg")), fmt);

		mShaderTransition = gl::GlslProg::create(loadAsset("common/shadertoy.vert"), loadAsset("common/shadertoy.frag"));
	}
	catch (const std::exception& e) {
		// Quit if anything went wrong.
		CI_LOG_EXCEPTION("Failed to load common textures and shaders:", e);
		quit(); return;
	}
	// instanciate the spout class
	mSpout = SpoutWrapper::create(mParameterBag, mBatchass->getTexturesRef());
	// instanciate the console class
	mConsole = AppConsole::create(mParameterBag, mBatchass);

	mTimer = 0.0f;

	// imgui
	margin = 3;
	inBetween = 3;
	// mPreviewFboWidth 80 mPreviewFboHeight 60 margin 10 inBetween 15 mPreviewWidth = 160;mPreviewHeight = 120;
	w = mParameterBag->mPreviewFboWidth + margin;
	h = mParameterBag->mPreviewFboHeight * 2.3;
	largeW = (mParameterBag->mPreviewFboWidth + margin) * 4;
	largeH = (mParameterBag->mPreviewFboHeight + margin) * 5;
	largePreviewW = mParameterBag->mPreviewWidth + margin;
	largePreviewH = (mParameterBag->mPreviewHeight + margin) * 2.4;
	displayHeight = mParameterBag->mMainDisplayHeight - 50;
	mouseGlobal = false;

	showConsole = showGlobal = showTextures = showAudio = showMidi = showChannels = showShaders = true;
	showTest = showTheme = showOSC = showFbos = false;

	// set ui window and io events callbacks
	ui::initialize();

	// RTE mBatchass->getShadersRef()->setupLiveShader();
	mBatchass->tapTempo();
}

void ReymentaMixnmapApp::cleanup()
{
	CI_LOG_V("shutdown");
	// save warp settings
	mBatchass->getWarpsRef()->save();
	// save params
	mParameterBag->save();
	ui::Shutdown();
	// close spout
	mSpout->shutdown();
	quit();
}

void ReymentaMixnmapApp::update()
{
	mSpout->update();
	mBatchass->update();
	//mAudio->update();
	mParameterBag->iFps = getAverageFps();
	mParameterBag->sFps = toString(floor(mParameterBag->iFps));
	getWindow()->setTitle(std::string("(" + mParameterBag->sFps + " fps)"));
}

void ReymentaMixnmapApp::draw()
{
	// must be first to avoid gl matrices to change
	// draw from Spout receivers
	mSpout->draw();
	// draw the fbos
	mBatchass->getTexturesRef()->draw();
	// Bind textures.
	if (mChannel0) mChannel0->bind(0);
	if (mChannel1) mChannel1->bind(1);
	if (mChannel2) mChannel2->bind(2);
	if (mChannel3) mChannel3->bind(3);

	// Render the current shader to a frame buffer.
	if ( mBatchass->getShadersRef()->getShaderCurrent() && mBufferCurrent) {
		gl::ScopedFramebuffer fbo(mBufferCurrent);

		// Bind shader.
		gl::ScopedGlslProg shader(mBatchass->getShadersRef()->getShaderCurrent());
		setUniforms();

		// Clear buffer and draw full screen quad (flipped).
		gl::clear();
		gl::drawSolidRect(Rectf(0, (float)getWindowHeight(), (float)getWindowWidth(), 0));
	}

	// Render the next shader to a frame buffer.
	if (mBatchass->getShadersRef()->getShaderNext() && mBufferNext) {
		gl::ScopedFramebuffer fbo(mBufferNext);

		// Bind shader.
		gl::ScopedGlslProg shader(mBatchass->getShadersRef()->getShaderNext());
		setUniforms();

		// Clear buffer and draw full screen quad (flipped).
		gl::clear();
		gl::drawSolidRect(Rectf(0, (float)getWindowHeight(), (float)getWindowWidth(), 0));
	}

	// Perform a cross-fade between the two shaders.
	double time = getElapsedSeconds() - mParameterBag->mTransitionTime;
	double fade = math<double>::clamp(time / mParameterBag->mTransitionDuration, 0.0, 1.0);

	if (fade <= 0.0) {
		// Transition has not yet started. Keep drawing current buffer.
		gl::draw(mBufferCurrent->getColorTexture(), getWindowBounds());
	}
	else if (fade < 1.0) {
		// Transition is in progress.
		// Use a transition shader to avoid having to draw one buffer on top of another.
		gl::ScopedTextureBind tex0(mBufferCurrent->getColorTexture(), 0);
		gl::ScopedTextureBind tex1(mBufferNext->getColorTexture(), 1);

		gl::ScopedGlslProg shader(mShaderTransition);
		mShaderTransition->uniform("iSrc", 0);
		mShaderTransition->uniform("iDst", 1);
		mShaderTransition->uniform("iFade", (float)fade);

		gl::drawSolidRect(getWindowBounds());
	}
	else if (mBatchass->getShadersRef()->getShaderNext()) {
		// Transition is done. Swap shaders.
		gl::draw(mBufferNext->getColorTexture(), getWindowBounds());
		mBatchass->getShadersRef()->swapShaders();
		
	}
	else {
		// No transition in progress.
		gl::draw(mBufferCurrent->getColorTexture(), getWindowBounds());
	}
	if (!mParameterBag->mShowUI || mBatchass->getWarpsRef()->isEditModeEnabled())
	{
		return;
	}
	//gl::enableAlphaBlending();
	//gl::setMatricesWindow(mParameterBag->mRenderWidth, mParameterBag->mRenderHeight);
	mBatchass->getWarpsRef()->draw();
	//gl::disableAlphaBlending();

	//imgui TO MIGRATE AND ADD FROM BatchassApp TOO
	if (!mParameterBag->mShowUI || mBatchass->getWarpsRef()->isEditModeEnabled())
	{
		return;
	}

	//gl::setViewport(getWindowBounds());
	gl::setMatricesWindow(getWindowSize());
	xPos = margin;
	yPos = margin;
	const char* warpInputs[] = { "mix", "left", "right", "warp1", "warp2", "preview", "abp", "live", "w8", "w9", "w10", "w11", "w12", "w13", "w14", "w15" };

#pragma region style
	// our theme variables
	ImGuiStyle& style = ui::GetStyle();
	style.WindowRounding = 4;
	style.WindowPadding = ImVec2(3, 3);
	style.FramePadding = ImVec2(2, 2);
	style.ItemSpacing = ImVec2(3, 3);
	style.ItemInnerSpacing = ImVec2(3, 3);
	style.WindowMinSize = ImVec2(w, mParameterBag->mPreviewFboHeight);
	style.Alpha = 0.6f;
	style.Colors[ImGuiCol_Text] = ImVec4(0.89f, 0.92f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.38f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.4f, 0.21f, 0.21f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.99f, 0.22f, 0.22f, 0.50f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.65f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.04f, 0.04f, 0.04f, 0.22f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.65f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.8f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.9f, 0.45f, 0.45f, 1.00f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.65f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.8f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.65f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.8f, 0.35f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TooltipBg] = ImVec4(0.65f, 0.25f, 0.25f, 1.00f);
#pragma endregion style

#pragma region mix
	// left/warp1 fbo
	ui::SetNextWindowSize(ImVec2(largePreviewW, largePreviewH), ImGuiSetCond_Once);
	ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
	ui::Begin("Warp1 fbo");
	{
		ui::PushItemWidth(mParameterBag->mPreviewFboWidth);
		ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.1f, 0.6f, 0.6f));
		ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.1f, 0.7f, 0.7f));
		ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.1f, 0.8f, 0.8f));

		sprintf_s(buf, "FV##f%d", 40);
		if (mParameterBag->mMode == mParameterBag->MODE_WARP)
		{
			ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mWarp1FboIndex), ivec2(mParameterBag->mPreviewWidth, mParameterBag->mPreviewHeight));
			if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboV(mParameterBag->mWarp1FboIndex);
			if (ui::IsItemHovered()) ui::SetTooltip("Flip vertically");
			// renderXY mouse
			ui::SliderFloat("W1RdrX", &mParameterBag->mWarp1RenderXY.x, -1.0f, 1.0f);
			ui::SliderFloat("W1RdrY", &mParameterBag->mWarp1RenderXY.y, -1.0f, 1.0f);
			// left zoom
			ui::SliderFloat("lZoom", &mParameterBag->iZoomLeft, mBatchass->minZoom, mBatchass->maxZoom);

			//ui::Columns(4);
			//ui::Text("ID"); ui::NextColumn();
			//ui::Text("idx"); ui::NextColumn();
			//ui::Text("mode"); ui::NextColumn();
			//ui::Text("actv"); ui::NextColumn();
			//ui::Separator();
			//for (int i = 0; i < mParameterBag->mWarpFbos.size() - 1; i++)
			//{
			//if (mParameterBag->mWarpFbos[i].textureIndex == 3)
			//{
			//ui::Text("%d", i); ui::NextColumn();
			//ui::Text("%d", mParameterBag->mWarpFbos[i].textureIndex); ui::NextColumn();
			//ui::Text("%d", mParameterBag->mWarpFbos[i].textureMode); ui::NextColumn();
			//ui::Text("%d", mParameterBag->mWarpFbos[i].active); ui::NextColumn();

			//}

			//}
			//ui::Columns(1);

		}
		else
		{
			ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mLeftFboIndex), ivec2(mParameterBag->mPreviewWidth, mParameterBag->mPreviewHeight));
			if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboV(mParameterBag->mLeftFboIndex);
			if (ui::IsItemHovered()) ui::SetTooltip("Flip vertically");
			// renderXY mouse
			ui::SliderFloat("LeftRdrX", &mParameterBag->mLeftRenderXY.x, -1.0f, 1.0f);
			ui::SliderFloat("LeftRdrY", &mParameterBag->mLeftRenderXY.y, -1.0f, 1.0f);
			// left zoom
			ui::SliderFloat("lZoom", &mParameterBag->iZoomLeft, mBatchass->minZoom, mBatchass->maxZoom);

			//ui::Columns(4);
			//ui::Text("ID"); ui::NextColumn();
			//ui::Text("idx"); ui::NextColumn();
			//ui::Text("mode"); ui::NextColumn();
			//ui::Text("actv"); ui::NextColumn();
			//ui::Separator();
			//for (int i = 0; i < mParameterBag->mWarpFbos.size() - 1; i++)
			//{
			//if (mParameterBag->mWarpFbos[i].textureIndex == 4)
			//{
			//ui::Text("%d", i); ui::NextColumn();
			//ui::Text("%d", mParameterBag->mWarpFbos[i].textureIndex); ui::NextColumn();
			//ui::Text("%d", mParameterBag->mWarpFbos[i].textureMode); ui::NextColumn();
			//ui::Text("%d", mParameterBag->mWarpFbos[i].active); ui::NextColumn();

			//}

			//}
			//ui::Columns(1);
		}
		ui::PopStyleColor(3);
		ui::PopItemWidth();
	}
	ui::End();
	xPos += largePreviewW + margin;

	// right/warp2 fbo
	ui::SetNextWindowSize(ImVec2(largePreviewW, largePreviewH), ImGuiSetCond_Once);
	ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
	ui::Begin("Warp2 fbo");
	{
		ui::PushItemWidth(mParameterBag->mPreviewFboWidth);

		ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.1f, 0.6f, 0.6f));
		ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.1f, 0.7f, 0.7f));
		ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.1f, 0.8f, 0.8f));

		sprintf_s(buf, "FV##f%d", 41);
		if (mParameterBag->mMode == mParameterBag->MODE_WARP)
		{
			ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mWarp2FboIndex), ivec2(mParameterBag->mPreviewWidth, mParameterBag->mPreviewHeight));
			if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboV(mParameterBag->mWarp2FboIndex);
			if (ui::IsItemHovered()) ui::SetTooltip("Flip vertically");
			// renderXY mouse
			ui::SliderFloat("W2RdrX", &mParameterBag->mWarp2RenderXY.x, -1.0f, 1.0f);
			ui::SliderFloat("W2RdrY", &mParameterBag->mWarp2RenderXY.y, -1.0f, 1.0f);
			// right zoom
			ui::SliderFloat("rZoom", &mParameterBag->iZoomRight, mBatchass->minZoom, mBatchass->maxZoom);
		}
		else
		{
			ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mRightFboIndex), ivec2(mParameterBag->mPreviewWidth, mParameterBag->mPreviewHeight));
			if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboV(mParameterBag->mRightFboIndex);
			if (ui::IsItemHovered()) ui::SetTooltip("Flip vertically");
			// renderXY mouse
			ui::SliderFloat("RightRdrX", &mParameterBag->mRightRenderXY.x, -1.0f, 1.0f);
			ui::SliderFloat("RightRdrY", &mParameterBag->mRightRenderXY.y, -1.0f, 1.0f);
			// right zoom
			ui::SliderFloat("rZoom", &mParameterBag->iZoomRight, mBatchass->minZoom, mBatchass->maxZoom);
		}
		ui::PopStyleColor(3);
		ui::PopItemWidth();
	}
	ui::End();
	xPos += largePreviewW + margin;

	// mix fbo
	ui::SetNextWindowSize(ImVec2(largePreviewW, largePreviewH), ImGuiSetCond_Once);
	ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
	ui::Begin("Mix fbo");
	{
		ui::PushItemWidth(mParameterBag->mPreviewFboWidth);


		ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mMixFboIndex), ivec2(mParameterBag->mPreviewWidth, mParameterBag->mPreviewHeight));
		ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.1f, 0.6f, 0.6f));
		ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.1f, 0.7f, 0.7f));
		ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.1f, 0.8f, 0.8f));

		sprintf_s(buf, "FV##fv%d", 42);
		if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboV(mParameterBag->mMixFboIndex);
		//mParameterBag->iFlipVertically ^= ui::Button(buf);
		if (ui::IsItemHovered()) ui::SetTooltip("Flip vertically");
		ui::SameLine();
		sprintf_s(buf, "FH##fh%d", 42);
		//mParameterBag->iFlipHorizontally ^= ui::Button(buf);
		if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboH(mParameterBag->mMixFboIndex);
		if (ui::IsItemHovered()) ui::SetTooltip("Flip horizontally");
		// crossfade
		if (ui::DragFloat("Xfade", &mParameterBag->controlValues[18], 0.01f, 0.001f, 1.0f))
		{
		}
		// renderXY mouse
		ui::DragFloat("RdrX", &mParameterBag->mRenderXY.x, 0.01f, -1.0f, 1.0f);
		ui::DragFloat("RdrY", &mParameterBag->mRenderXY.y, 0.01f, -1.0f, 1.0f);
		ui::PopStyleColor(3);
		ui::PopItemWidth();

	}
	ui::End();
	xPos += largePreviewW + margin;

	// preview fbo
	ui::SetNextWindowSize(ImVec2(largePreviewW, largePreviewH), ImGuiSetCond_Once);
	ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
	ui::Begin("Preview fbo");
	{
		ui::PushItemWidth(mParameterBag->mPreviewFboWidth);

		ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mCurrentPreviewFboIndex), ivec2(mParameterBag->mPreviewWidth, mParameterBag->mPreviewHeight));
		ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.1f, 0.6f, 0.6f));
		ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.1f, 0.7f, 0.7f));
		ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.1f, 0.8f, 0.8f));

		if (mParameterBag->mPreviewEnabled)
		{
			sprintf_s(buf, "On##pvwe");
		}
		else
		{
			sprintf_s(buf, "Off##pvwe");
		}
		mParameterBag->mPreviewEnabled ^= ui::Button(buf);
		if (ui::IsItemHovered()) ui::SetTooltip("Preview enabled");
		ui::SameLine();
		sprintf_s(buf, "Reset##pvreset");
		if (ui::Button(buf)) mParameterBag->reset();
		if (ui::IsItemHovered()) ui::SetTooltip("Reset live params");
		ui::PopStyleColor(3);
		ui::Text(mBatchass->getTexturesRef()->getPreviewTime());

		ui::PopItemWidth();

	}
	ui::End();
	xPos += largePreviewW + margin;
#pragma endregion mix

#pragma region channels
	if (showChannels)
	{
		ui::SetNextWindowSize(ImVec2(w * 2, largePreviewH), ImGuiSetCond_Once);
		ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);

		ui::Begin("Channels");
		{
			ui::Columns(3);
			ui::SetColumnOffset(0, 4.0f);// int column_index, float offset)
			ui::SetColumnOffset(1, 20.0f);// int column_index, float offset)
			//ui::SetColumnOffset(2, 24.0f);// int column_index, float offset)
			ui::Text("Chn"); ui::NextColumn();
			ui::Text("Tex"); ui::NextColumn();
			ui::Text("Name"); ui::NextColumn();
			ui::Separator();
			for (int i = 0; i < mParameterBag->MAX - 1; i++)
			{
				ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
				ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
				ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
				ui::Text("c%d", i);
				ui::NextColumn();
				sprintf_s(buf, "%d", i);
				if (ui::SliderInt(buf, &mParameterBag->iChannels[i], 0, mParameterBag->MAX - 1)) {
				}
				ui::NextColumn();
				ui::PopStyleColor(3);
				ui::Text("%s", mBatchass->getTexturesRef()->getTextureName(mParameterBag->iChannels[i]));
				ui::NextColumn();
			}
			ui::Columns(1);
		}
		ui::End();
		xPos += w * 2 + margin;
	}
#pragma endregion channels

#pragma region Info

	ui::SetNextWindowSize(ImVec2(largePreviewW + 20, largePreviewH), ImGuiSetCond_Once);
	ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
	sprintf_s(buf, "Fps %c %d###fps", "|/-\\"[(int)(ImGui::GetTime() / 0.25f) & 3], (int)mParameterBag->iFps);
	ui::Begin(buf);
	{
		ImGui::PushItemWidth(mParameterBag->mPreviewFboWidth);
		// fps
		static ImVector<float> values; if (values.empty()) { values.resize(100); memset(&values.front(), 0, values.size()*sizeof(float)); }
		static int values_offset = 0;
		static float refresh_time = -1.0f;
		if (ui::GetTime() > refresh_time + 1.0f / 6.0f)
		{
			refresh_time = ui::GetTime();
			values[values_offset] = mParameterBag->iFps;
			values_offset = (values_offset + 1) % values.size();
		}
		if (mParameterBag->iFps < 12.0) ui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
		ui::PlotLines("FPS", &values.front(), (int)values.size(), values_offset, mParameterBag->sFps.c_str(), 0.0f, 300.0f, ImVec2(0, 30));
		if (mParameterBag->iFps < 12.0) ui::PopStyleColor();

		// Checkbox
		ui::Checkbox("Tex", &showTextures);
		ui::SameLine();
		ui::Checkbox("Fbos", &showFbos);
		ui::SameLine();
		ui::Checkbox("Shada", &showShaders);

		ui::Checkbox("Audio", &showAudio);
		ui::SameLine();
		ui::Checkbox("Cmd", &showConsole);
		ui::SameLine();
		ui::Checkbox("OSC", &showOSC);

		ui::Checkbox("MIDI", &showMidi);
		ui::SameLine();
		ui::Checkbox("Test", &showTest);
		if (ui::Button("Save Params"))
		{
			// save warp settings
			mBatchass->getWarpsRef()->save("warps1.xml");
			// save params
			mParameterBag->save();
		}

		mParameterBag->iDebug ^= ui::Button("Debug");
		ui::SameLine();
		mParameterBag->mRenderThumbs ^= ui::Button("Thumbs");
		ui::PopItemWidth();
		if (ui::Button("Stop Loading")) mBatchass->stopLoading();
	}
	ui::End();
	xPos += largePreviewW + 20 + margin;

#pragma endregion Info

#pragma region Audio

	// audio window
	if (showAudio)
	{
		ui::SetNextWindowSize(ImVec2(largePreviewW + 20, largePreviewH), ImGuiSetCond_Once);
		ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
		ui::Begin("Audio##ap");
		{
			ui::Text("Beat %d", mParameterBag->iBeat);
			ui::SameLine();
			ui::Text("Time %.2f", mParameterBag->iGlobalTime);
			//ui::Checkbox("Playing", &mParameterBag->mIsPlaying);

			ui::Text("Tempo %.2f", mParameterBag->mTempo);
			if (ui::Button("Tap tempo")) { mBatchass->tapTempo(); }
			ui::SameLine();
			if (ui::Button("Time tempo")) { mParameterBag->mUseTimeWithTempo = !mParameterBag->mUseTimeWithTempo; }

			//void Batchass::setTimeFactor(const int &aTimeFactor)
			ui::SliderFloat("time x", &mParameterBag->iTimeFactor, 0.0001f, 32.0f, "%.1f");

			static ImVector<float> values; if (values.empty()) { values.resize(40); memset(&values.front(), 0, values.size()*sizeof(float)); }
			static int values_offset = 0;
			// audio maxVolume
			static float refresh_time = -1.0f;
			if (ui::GetTime() > refresh_time + 1.0f / 20.0f)
			{
				refresh_time = ui::GetTime();
				values[values_offset] = mParameterBag->maxVolume;
				values_offset = (values_offset + 1) % values.size();
			}

			ui::SliderFloat("mult x", &mParameterBag->controlValues[13], 0.01f, 10.0f);
			//ImGui::PlotHistogram("Histogram", mAudio->getSmallSpectrum(), 7, 0, NULL, 0.0f, 255.0f, ImVec2(0, 30));

			if (mParameterBag->maxVolume > 240.0) ui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ui::PlotLines("Volume", &values.front(), (int)values.size(), values_offset, toString(mBatchass->formatFloat(mParameterBag->maxVolume)).c_str(), 0.0f, 255.0f, ImVec2(0, 30));
			if (mParameterBag->maxVolume > 240.0) ui::PopStyleColor();
			ui::Text("Track %s %.2f", mParameterBag->mTrackName.c_str(), mParameterBag->liveMeter);

			if (ui::Button("x##spdx")) { mParameterBag->iSpeedMultiplier = 1.0; }
			ui::SameLine();
			ui::SliderFloat("speed x", &mParameterBag->iSpeedMultiplier, 0.01f, 5.0f, "%.1f");

		}
		ui::End();
		xPos += largePreviewW + 20 + margin;
		//yPos += largePreviewH + margin;
	}
#pragma endregion Audio

#pragma region MIDI

	// MIDI window
	if (showMidi)
	{
		ui::SetNextWindowSize(ImVec2(largePreviewW + 20, largePreviewH), ImGuiSetCond_Once);
		ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
		ui::Begin("MIDI");
		{
			sprintf_s(buf, "Enable");
			if (ui::Button(buf)) mBatchass->midiSetup();
			if (ui::CollapsingHeader("MidiIn", "20", true, true))
			{
				ui::Columns(2, "data", true);
				ui::Text("Name"); ui::NextColumn();
				ui::Text("Connect"); ui::NextColumn();
				ui::Separator();

				for (int i = 0; i < mBatchass->midiInCount(); i++)
				{
					ui::Text(mBatchass->midiInPortName(i).c_str()); ui::NextColumn();

					if (mBatchass->midiInConnected(i))
					{
						sprintf_s(buf, "Disconnect %d", i);
					}
					else
					{
						sprintf_s(buf, "Connect %d", i);
					}

					if (ui::Button(buf))
					{
						if (mBatchass->midiInConnected(i))
						{
							mBatchass->midiInClosePort(i);
						}
						else
						{
							mBatchass->midiInOpenPort(i);
						}
					}
					ui::NextColumn();
					ui::Separator();
				}
				ui::Columns(1);
			}
		}
		ui::End();
		xPos += largePreviewW + 20 + margin;
		yPos = margin;

	}
#pragma endregion MIDI

#pragma region Global

	ui::SetNextWindowSize(ImVec2(largeW, displayHeight), ImGuiSetCond_Once);
	ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
	ui::Begin("Animation");
	{
		ImGui::PushItemWidth(mParameterBag->mPreviewFboWidth);

		if (ui::CollapsingHeader("Mouse", NULL, true, true))
		{
			ui::Text("Mouse Position: (%.1f,%.1f)", ui::GetIO().MousePos.x, ui::GetIO().MousePos.y); ui::SameLine();
			ui::Text("Clic %d", ui::GetIO().MouseDown[0]);
			mouseGlobal ^= ui::Button("mouse gbl");
			if (mouseGlobal)
			{
				mParameterBag->mRenderPosXY.x = ui::GetIO().MousePos.x; ui::SameLine();
				mParameterBag->mRenderPosXY.y = ui::GetIO().MousePos.y;
				mParameterBag->iMouse.z = ui::GetIO().MouseDown[0];
			}
			else
			{

				mParameterBag->iMouse.z = ui::Button("mouse click");
			}
			ui::SliderFloat("MouseX", &mParameterBag->mRenderPosXY.x, 0, mParameterBag->mFboWidth);
			ui::SliderFloat("MouseY", &mParameterBag->mRenderPosXY.y, 0, 2048);// mParameterBag->mFboHeight);

		}
		if (ui::CollapsingHeader("Effects", NULL, true, true))
		{
			int hue = 0;

			(mParameterBag->iRepeat) ? ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue / 7.0f, 1.0f, 0.5f)) : ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1.0f, 0.1f, 0.1f));
			ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue / 7.0f, 0.7f, 0.7f));
			ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue / 7.0f, 0.8f, 0.8f));
			mParameterBag->iRepeat ^= ui::Button("repeat");
			ui::PopStyleColor(3);
			hue++;
			ui::SameLine();

			(mParameterBag->controlValues[45]) ? ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue / 7.0f, 1.0f, 0.5f)) : ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1.0f, 0.1f, 0.1f));
			ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue / 7.0f, 0.7f, 0.7f));
			ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue / 7.0f, 0.8f, 0.8f));
			if (ui::Button("glitch")) { mParameterBag->controlValues[45] = !mParameterBag->controlValues[45]; }
			ui::PopStyleColor(3);
			hue++;
			ui::SameLine();

			(mParameterBag->controlValues[46]) ? ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue / 7.0f, 1.0f, 0.5f)) : ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1.0f, 0.1f, 0.1f));
			ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue / 7.0f, 0.7f, 0.7f));
			ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue / 7.0f, 0.8f, 0.8f));
			if (ui::Button("toggle")) { mParameterBag->controlValues[46] = !mParameterBag->controlValues[46]; }
			ui::PopStyleColor(3);
			hue++;
			ui::SameLine();

			(mParameterBag->controlValues[47]) ? ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue / 7.0f, 1.0f, 0.5f)) : ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1.0f, 0.1f, 0.1f));
			ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue / 7.0f, 0.7f, 0.7f));
			ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue / 7.0f, 0.8f, 0.8f));
			if (ui::Button("vignette")) { mParameterBag->controlValues[47] = !mParameterBag->controlValues[47]; }
			ui::PopStyleColor(3);
			hue++;

			(mParameterBag->controlValues[48]) ? ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue / 7.0f, 1.0f, 0.5f)) : ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1.0f, 0.1f, 0.1f));
			ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue / 7.0f, 0.7f, 0.7f));
			ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue / 7.0f, 0.8f, 0.8f));
			if (ui::Button("invert")) { mParameterBag->controlValues[48] = !mParameterBag->controlValues[48]; }
			ui::PopStyleColor(3);
			hue++;
			ui::SameLine();

			(mParameterBag->iGreyScale) ? ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(hue / 7.0f, 1.0f, 0.5f)) : ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1.0f, 0.1f, 0.1f));
			ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(hue / 7.0f, 0.7f, 0.7f));
			ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(hue / 7.0f, 0.8f, 0.8f));
			mParameterBag->iGreyScale ^= ui::Button("greyscale");
			ui::PopStyleColor(3);
			hue++;
			ui::SameLine();

			if (ui::Button("blackout"))
			{
				mParameterBag->controlValues[1] = mParameterBag->controlValues[2] = mParameterBag->controlValues[3] = mParameterBag->controlValues[4] = 0.0;
				mParameterBag->controlValues[5] = mParameterBag->controlValues[6] = mParameterBag->controlValues[7] = mParameterBag->controlValues[8] = 0.0;
			}
		}
		if (ui::CollapsingHeader("Animation", NULL, true, true))
		{

			ui::SliderInt("mUIRefresh", &mParameterBag->mUIRefresh, 1, 255);
			int ctrl;
			stringstream aParams;
			aParams << "{\"params\" :[{\"name\" : 0,\"value\" : " << getElapsedFrames() << "}"; // TimeStamp

			// iChromatic
			ctrl = 10;
			if (ui::Button("a##chromatic")) { mBatchass->lockChromatic(); }
			ui::SameLine();
			if (ui::Button("t##chromatic")) { mBatchass->tempoChromatic(); }
			ui::SameLine();
			if (ui::Button("x##chromatic")) { mBatchass->resetChromatic(); }
			ui::SameLine();
			if (ui::SliderFloat("chromatic/min/max", &mParameterBag->controlValues[ctrl], mBatchass->minChromatic, mBatchass->maxChromatic))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}

			// ratio
			ctrl = 11;
			if (ui::Button("a##ratio")) { mBatchass->lockRatio(); }
			ui::SameLine();
			if (ui::Button("t##ratio")) { mBatchass->tempoRatio(); }
			ui::SameLine();
			if (ui::Button("x##ratio")) { mBatchass->resetRatio(); }
			ui::SameLine();
			if (ui::SliderFloat("ratio/min/max", &mParameterBag->controlValues[ctrl], mBatchass->minRatio, mBatchass->maxRatio))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}
			// exposure
			ctrl = 14;
			if (ui::Button("a##exposure")) { mBatchass->lockExposure(); }
			ui::SameLine();
			if (ui::Button("t##exposure")) { mBatchass->tempoExposure(); }
			ui::SameLine();
			if (ui::Button("x##exposure")) { mBatchass->resetExposure(); }
			ui::SameLine();
			if (ui::DragFloat("exposure", &mParameterBag->controlValues[ctrl], 0.1f, mBatchass->minExposure, mParameterBag->maxExposure))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}

			// zoom
			ctrl = 22;
			if (ui::Button("a##zoom"))
			{
				mBatchass->lockZoom();
			}
			ui::SameLine();
			if (ui::Button("t##zoom")) { mBatchass->tempoZoom(); }
			ui::SameLine();
			if (ui::Button("x##zoom")) { mBatchass->resetZoom(); }
			ui::SameLine();
			if (ui::DragFloat("zoom", &mParameterBag->controlValues[ctrl], 0.1f, mBatchass->minZoom, mBatchass->maxZoom))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}
			// z position
			ctrl = 9;
			if (ui::Button("a##zpos")) { mBatchass->lockZPos(); }
			ui::SameLine();
			if (ui::Button("t##zpos")) { mBatchass->tempoZPos(); }
			ui::SameLine();
			if (ui::Button("x##zpos")) { mBatchass->resetZPos(); }
			ui::SameLine();
			if (ui::SliderFloat("zPosition", &mParameterBag->controlValues[ctrl], mBatchass->minZPos, mBatchass->maxZPos))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}

			// rotation speed 
			ctrl = 19;
			if (ui::Button("a##rotationspeed")) { mBatchass->lockRotationSpeed(); }
			ui::SameLine();
			if (ui::Button("t##rotationspeed")) { mBatchass->tempoRotationSpeed(); }
			ui::SameLine();
			if (ui::Button("x##rotationspeed")) { mBatchass->resetRotationSpeed(); }
			ui::SameLine();
			if (ui::DragFloat("rotationSpeed", &mParameterBag->controlValues[ctrl], 0.01f, mBatchass->minRotationSpeed, mBatchass->maxRotationSpeed))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}
			// badTv
			if (ui::Button("x##badtv")) { mParameterBag->iBadTv = 0.0f; }
			ui::SameLine();
			if (ui::SliderFloat("badTv/min/max", &mParameterBag->iBadTv, 0.0f, 5.0f))
			{
			}
			// param1
			if (ui::Button("x##param1")) { mParameterBag->iParam1 = 1.0f; }
			ui::SameLine();
			if (ui::SliderFloat("param1/min/max", &mParameterBag->iParam1, 0.01f, 100.0f))
			{
			}
			// param2
			if (ui::Button("x##param2")) { mParameterBag->iParam2 = 1.0f; }
			ui::SameLine();
			if (ui::SliderFloat("param2/min/max", &mParameterBag->iParam2, 0.01f, 100.0f))
			{
			}
			sprintf_s(buf, "XorY");
			mParameterBag->iXorY ^= ui::Button(buf);
			// blend modes
			if (ui::Button("x##blendmode")) { mParameterBag->iBlendMode = 0.0f; }
			ui::SameLine();
			ui::SliderInt("blendmode", &mParameterBag->iBlendMode, 0, mParameterBag->maxBlendMode);

			// steps
			ctrl = 20;
			if (ui::Button("x##steps")) { mParameterBag->controlValues[ctrl] = 16.0f; }
			ui::SameLine();
			if (ui::SliderFloat("steps", &mParameterBag->controlValues[ctrl], 1.0f, 128.0f))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}
			// pixelate
			ctrl = 15;
			if (ui::Button("x##pixelate")) { mParameterBag->controlValues[ctrl] = 1.0f; }
			ui::SameLine();
			if (ui::SliderFloat("pixelate", &mParameterBag->controlValues[ctrl], 0.01f, 1.0f))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}
			// trixels
			ctrl = 16;
			if (ui::Button("x##trixels")) { mParameterBag->controlValues[ctrl] = 0.0f; }
			ui::SameLine();
			if (ui::SliderFloat("trixels", &mParameterBag->controlValues[ctrl], 0.00f, 1.0f))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}
			// grid
			ctrl = 17;
			if (ui::Button("x##grid")) { mParameterBag->controlValues[ctrl] = 0.0f; }
			ui::SameLine();
			if (ui::SliderFloat("grid", &mParameterBag->controlValues[ctrl], 0.00f, 60.0f))
			{
				aParams << ",{\"name\" : " << ctrl << ",\"value\" : " << mParameterBag->controlValues[ctrl] << "}";
			}

			aParams << "]}";
			string strAParams = aParams.str();
			if (strAParams.length() > 60)
			{
				mBatchass->sendJSON(strAParams);

			}
		}
		ui::PopItemWidth();
		if (ui::CollapsingHeader("Colors", NULL, true, true))
		{
			stringstream sParams;
			bool colorChanged = false;
			sParams << "{\"params\" :[{\"name\" : 0,\"value\" : " << getElapsedFrames() << "}"; // TimeStamp
			// foreground color
			color[0] = mParameterBag->controlValues[1];
			color[1] = mParameterBag->controlValues[2];
			color[2] = mParameterBag->controlValues[3];
			color[3] = mParameterBag->controlValues[4];
			ui::ColorEdit4("f", color);

			for (int i = 0; i < 4; i++)
			{
				if (mParameterBag->controlValues[i + 1] != color[i])
				{
					sParams << ",{\"name\" : " << i + 1 << ",\"value\" : " << color[i] << "}";
					mParameterBag->controlValues[i + 1] = color[i];
					colorChanged = true;
				}
			}
			if (colorChanged) mBatchass->colorWrite(); //lights4events

			// background color
			backcolor[0] = mParameterBag->controlValues[5];
			backcolor[1] = mParameterBag->controlValues[6];
			backcolor[2] = mParameterBag->controlValues[7];
			backcolor[3] = mParameterBag->controlValues[8];
			ui::ColorEdit4("g", backcolor);
			for (int i = 0; i < 4; i++)
			{
				if (mParameterBag->controlValues[i + 5] != backcolor[i])
				{
					sParams << ",{\"name\" : " << i + 5 << ",\"value\" : " << backcolor[i] << "}";
					mParameterBag->controlValues[i + 5] = backcolor[i];
				}

			}
			// color multipliers
			if (ui::Button("x##RedX")) { mParameterBag->iRedMultiplier = 1.0f; }
			ui::SameLine();
			if (ui::SliderFloat("RedX", &mParameterBag->iRedMultiplier, 0.0f, 3.0f))
			{
			}
			if (ui::Button("x##GreenX")) { mParameterBag->iGreenMultiplier = 1.0f; }
			ui::SameLine();
			if (ui::SliderFloat("GreenX", &mParameterBag->iGreenMultiplier, 0.0f, 3.0f))
			{
			}
			if (ui::Button("x##BlueX")) { mParameterBag->iBlueMultiplier = 1.0f; }
			ui::SameLine();
			if (ui::SliderFloat("BlueX", &mParameterBag->iBlueMultiplier, 0.0f, 3.0f))
			{
			}

			sParams << "]}";
			string strParams = sParams.str();
			if (strParams.length() > 60)
			{
				mBatchass->sendJSON(strParams);
			}

		}

		if (ui::CollapsingHeader("Camera", NULL, true, true))
		{
			ui::SliderFloat("Pos.x", &mParameterBag->mRenderPosXY.x, 0.0f, mParameterBag->mRenderWidth);
			ui::SliderFloat("Pos.y", &mParameterBag->mRenderPosXY.y, 0.0f, mParameterBag->mRenderHeight);
			float eyeZ = mParameterBag->mCamera.getEyePoint().z;
			if (ui::SliderFloat("Eye.z", &eyeZ, -500.0f, 1.0f))
			{
				vec3 eye = mParameterBag->mCamera.getEyePoint();
				eye.z = eyeZ;
				mParameterBag->mCamera.setEyePoint(eye);
			}
			ui::SliderFloat("ABP Bend", &mParameterBag->mBend, -20.0f, 20.0f);

		}
		if (ui::CollapsingHeader("Tracks", NULL, true, true))
		{
			for (int a = 0; a < mParameterBag->MAX; a++)
			{
				if (mBatchass->getTrack(a) != "default.glsl") ui::Button(mBatchass->getTrack(a).c_str());
			}
		}
	}
	ui::End();

#pragma endregion Global
	// next line
	xPos = margin;
	yPos += largePreviewH + margin;

#pragma region warps
	if (mParameterBag->mMode == MODE_WARP)
	{
		for (int i = 0; i < mBatchass->getWarpsRef()->getWarpsCount(); i++)
		{
			sprintf_s(buf, "Warp %d", i);
			ui::SetNextWindowSize(ImVec2(w, h));
			ui::Begin(buf);
			{
				ui::SetWindowPos(ImVec2((i * (w + inBetween)) + margin, yPos));
				ui::PushID(i);
				ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(mParameterBag->mWarpFbos[i].textureIndex), ivec2(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight));
				ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
				ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
				ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
				sprintf_s(buf, "%d", mParameterBag->mWarpFbos[i].textureIndex);
				if (ui::SliderInt(buf, &mParameterBag->mWarpFbos[i].textureIndex, 0, mParameterBag->MAX - 1)) {
				}
				sprintf_s(buf, "%s", warpInputs[mParameterBag->mWarpFbos[i].textureIndex]);
				ui::Text(buf);

				ui::PopStyleColor(3);
				ui::PopID();
			}
			ui::End();
		}
		yPos += h + margin;
	}
#pragma endregion warps

#pragma region textures
	if (showTextures)
	{
		for (int i = 0; i < mBatchass->getTexturesRef()->getTextureCount(); i++)
		{
			ui::SetNextWindowSize(ImVec2(w, h*1.4));
			ui::SetNextWindowPos(ImVec2((i * (w + inBetween)) + margin, yPos));
			//ui::Begin(textureNames[i], NULL, ImVec2(0, 0), ui::GetStyle().Alpha, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
			ui::Begin(mBatchass->getTexturesRef()->getTextureName(i), NULL, ImVec2(0, 0), ui::GetStyle().Alpha, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
			{
				ui::PushID(i);
				ui::Image((void*)mBatchass->getTexturesRef()->getTexture(i)->getId(), ivec2(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight));
				ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
				ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
				ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
				//BEGIN
				sprintf_s(buf, "WS##s%d", i);
				if (ui::Button(buf))
				{
					sprintf_s(buf, "IMG=%d.jpg", i);
					//mBatchass->wsWrite(buf);
				}
				if (ui::IsItemHovered()) ui::SetTooltip("Send texture file name via WebSockets");
				ui::SameLine();
				sprintf_s(buf, "FV##s%d", i);
				if (ui::Button(buf))
				{
					mBatchass->getTexturesRef()->flipTexture(i);
				}
				if (mBatchass->getTexturesRef()->isSequence(i)) {
					if (!mBatchass->getTexturesRef()->isLoadingFromDisk(i)) {
						ui::SameLine();
						sprintf_s(buf, "LD##s%d", i);
						if (ui::Button(buf))
						{
							mBatchass->getTexturesRef()->toggleLoadingFromDisk(i);
						}
						if (ui::IsItemHovered()) ui::SetTooltip("Pause loading from disk");
					}
					sprintf_s(buf, ">##s%d", i);
					if (ui::Button(buf))
					{
						mBatchass->getTexturesRef()->playSequence(i);
					}
					ui::SameLine();
					sprintf_s(buf, "\"##s%d", i);
					if (ui::Button(buf))
					{
						mBatchass->getTexturesRef()->pauseSequence(i);
					}
					ui::SameLine();
					sprintf_s(buf, "r##s%d", i);
					if (ui::Button(buf))
					{
						mBatchass->getTexturesRef()->reverseSequence(i);
					}
					ui::SameLine();
					playheadPositions[i] = mBatchass->getTexturesRef()->getPlayheadPosition(i);
					sprintf_s(buf, "p%d##s%d", playheadPositions[i], i);
					if (ui::Button(buf))
					{
						mBatchass->getTexturesRef()->setPlayheadPosition(i, 0);
					}

					if (ui::SliderInt("scrub", &playheadPositions[i], 0, mBatchass->getTexturesRef()->getMaxFrames(i)))
					{
						mBatchass->getTexturesRef()->setPlayheadPosition(i, playheadPositions[i]);
					}
					speeds[i] = mBatchass->getTexturesRef()->getSpeed(i);
					if (ui::SliderFloat("speed", &speeds[i], 0.0f, 6.0f))
					{
						mBatchass->getTexturesRef()->setSpeed(i, speeds[i]);
					}

				}

				//END
				ui::PopStyleColor(3);
				ui::PopID();
			}
			ui::End();
		}
		yPos += h*1.4 + margin;
	}
#pragma endregion textures

#pragma region library
	if (showShaders)
	{

		static ImGuiTextFilter filter;
		ui::Text("Filter usage:\n"
			"  \"\"         display all lines\n"
			"  \"xxx\"      display lines containing \"xxx\"\n"
			"  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
			"  \"-xxx\"     hide lines containing \"xxx\"");
		filter.Draw();


		for (int i = 0; i < mBatchass->getShadersRef()->getCount(); i++)
		{
			if (filter.PassFilter(mBatchass->getShadersRef()->getShader(i).name.c_str()))
				ui::BulletText("%s", mBatchass->getShadersRef()->getShader(i).name.c_str());
		}

		xPos = margin;
		for (int i = 0; i < mBatchass->getShadersRef()->getCount(); i++)
		{
			if (filter.PassFilter(mBatchass->getShadersRef()->getShader(i).name.c_str()) && mBatchass->getShadersRef()->getShader(i).active)
			{

				sprintf_s(buf, "%d##lsh%d", mBatchass->getShadersRef()->getShader(i).microseconds, i);
				ui::SetNextWindowSize(ImVec2(w, h));
				ui::SetNextWindowPos(ImVec2(xPos + margin, yPos));
				ui::Begin(buf, NULL, ImVec2(0, 0), ui::GetStyle().Alpha, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
				{
					xPos += w + inBetween;
					if (xPos > mParameterBag->MAX * w * 1.0)
					{
						xPos = margin;
						yPos += h + margin;
					}
					ui::PushID(i);
					ui::Image((void*)mBatchass->getTexturesRef()->getShaderThumbTextureId(i), ivec2(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight));
					if (ui::IsItemHovered()) ui::SetTooltip(mBatchass->getShadersRef()->getShader(i).name.c_str());

					//ui::Columns(2, "lr", false);
					// left
					if (mParameterBag->mLeftFragIndex == i)
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 1.0f, 0.5f));
					}
					else
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.1f, 0.1f));

					}
					ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.0f, 0.7f, 0.7f));
					ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.0f, 0.8f, 0.8f));
					sprintf_s(buf, "L##s%d", i);
					if (ui::Button(buf)) mBatchass->selectShader(true, i);
					if (ui::IsItemHovered()) ui::SetTooltip("Set shader to left");
					ui::PopStyleColor(3);
					//ui::NextColumn();
					ui::SameLine();
					// right
					if (mParameterBag->mRightFragIndex == i)
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.3f, 1.0f, 0.5f));
					}
					else
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.1f, 0.1f));
					}
					ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.3f, 0.7f, 0.7f));
					ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.3f, 0.8f, 0.8f));
					sprintf_s(buf, "R##s%d", i);
					if (ui::Button(buf))  mBatchass->selectShader(false, i);
					if (ui::IsItemHovered()) ui::SetTooltip("Set shader to right");
					ui::PopStyleColor(3);
					//ui::NextColumn();
					ui::SameLine();
					// preview
					if (mParameterBag->mPreviewFragIndex == i)
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.6f, 1.0f, 0.5f));
					}
					else
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.1f, 0.1f));
					}
					ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.6f, 0.7f, 0.7f));
					ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.6f, 0.8f, 0.8f));
					sprintf_s(buf, "P##s%d", i);
					if (ui::Button(buf)) mParameterBag->mPreviewFragIndex = i;
					if (ui::IsItemHovered()) ui::SetTooltip("Preview shader");
					ui::PopStyleColor(3);

					// warp1
					if (mParameterBag->mWarp1FragIndex == i)
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.16f, 1.0f, 0.5f));
					}
					else
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.1f, 0.1f));
					}
					ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.16f, 0.7f, 0.7f));
					ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.16f, 0.8f, 0.8f));
					sprintf_s(buf, "1##s%d", i);
					if (ui::Button(buf)) mParameterBag->mWarp1FragIndex = i;
					if (ui::IsItemHovered()) ui::SetTooltip("Set warp 1 shader");
					ui::PopStyleColor(3);
					ui::SameLine();

					// warp2
					if (mParameterBag->mWarp2FragIndex == i)
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.77f, 1.0f, 0.5f));
					}
					else
					{
						ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0.0f, 0.1f, 0.1f));
					}
					ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0.77f, 0.7f, 0.7f));
					ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0.77f, 0.8f, 0.8f));
					sprintf_s(buf, "2##s%d", i);
					if (ui::Button(buf)) mParameterBag->mWarp2FragIndex = i;
					if (ui::IsItemHovered()) ui::SetTooltip("Set warp 2 shader");
					ui::PopStyleColor(3);

					// enable removing shaders
					if (i > 4)
					{
						ui::SameLine();
						sprintf_s(buf, "X##s%d", i);
						if (ui::Button(buf)) mBatchass->getShadersRef()->removePixelFragmentShaderAtIndex(i);
						if (ui::IsItemHovered()) ui::SetTooltip("Remove shader");
					}

					ui::PopID();

				}
				ui::End();
			} // if filtered

		} // for
		xPos = margin;
		yPos += h + margin;
	}
#pragma endregion library

#pragma region fbos

	if (showFbos)
	{
		for (int i = 0; i < mBatchass->getTexturesRef()->getFboCount(); i++)
		{
			ui::SetNextWindowSize(ImVec2(w, h));
			ui::SetNextWindowPos(ImVec2((i * (w + inBetween)) + margin, yPos));
			ui::Begin(mBatchass->getTexturesRef()->getFboName(i), NULL, ImVec2(0, 0), ui::GetStyle().Alpha, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
			{
				//if (i > 0) ui::SameLine();
				ui::PushID(i);
				ui::Image((void*)mBatchass->getTexturesRef()->getFboTextureId(i), ivec2(mParameterBag->mPreviewFboWidth, mParameterBag->mPreviewFboHeight));
				ui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
				ui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
				ui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(i / 7.0f, 0.8f, 0.8f));

				sprintf_s(buf, "FV##f%d", i);
				if (ui::Button(buf)) mBatchass->getTexturesRef()->flipFboV(i);
				if (ui::IsItemHovered()) ui::SetTooltip("Flip vertically");

				ui::PopStyleColor(3);
				ui::PopID();
			}
			ui::End();
		}
		yPos += h + margin;
	}
#pragma endregion fbos
	// console
	if (showConsole)
	{
		ui::SetNextWindowSize(ImVec2((w + margin) * mParameterBag->MAX, largePreviewH), ImGuiSetCond_Once);
		ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
		ShowAppConsole(&showConsole);
		if (mParameterBag->newMsg)
		{
			mParameterBag->newMsg = false;
			mConsole->AddLog(mParameterBag->mMsg.c_str());
		}
	}
	if (showTest)
	{
		ui::ShowTestWindow();
		ui::ShowStyleEditor();

	}
	xPos += largePreviewH + margin;

#pragma region OSC

	if (showOSC && mParameterBag->mOSCEnabled)
	{
		ui::SetNextWindowSize(ImVec2(largeW, largeH), ImGuiSetCond_Once);
		ui::SetNextWindowPos(ImVec2(xPos, yPos), ImGuiSetCond_Once);
		ui::Begin("OSC router");
		{
			ui::Text("Sending to host %s", mParameterBag->mOSCDestinationHost.c_str());
			ui::SameLine();
			ui::Text(" on port %d", mParameterBag->mOSCDestinationPort);
			ui::Text("Sending to 2nd host %s", mParameterBag->mOSCDestinationHost2.c_str());
			ui::SameLine();
			ui::Text(" on port %d", mParameterBag->mOSCDestinationPort2);
			ui::Text(" Receiving on port %d", mParameterBag->mOSCReceiverPort);

			static char str0[128] = "/live/play";
			static int i0 = 0;
			static float f0 = 0.0f;
			ui::InputText("address", str0, IM_ARRAYSIZE(str0));
			ui::InputInt("track", &i0);
			ui::InputFloat("clip", &f0, 0.01f, 1.0f);
			if (ui::Button("Send")) { mBatchass->sendOSCIntMessage(str0, i0); }
		}
		ui::End();
		xPos += largeW + margin;
	}
#pragma endregion OSC

	//gl::disableAlphaBlending();
}
void ReymentaMixnmapApp::mouseMove(MouseEvent event)
{
	if (mParameterBag->mMode == mParameterBag->MODE_WARP) mBatchass->getWarpsRef()->mouseMove(event);
}
void ReymentaMixnmapApp::mouseDown(MouseEvent event)
{
	mMouse.x = (float)event.getPos().x;
	mMouse.y = (float)event.getPos().y;
	mMouse.z = (float)event.getPos().x;
	mMouse.w = (float)event.getPos().y;
	if (mParameterBag->mMode == mParameterBag->MODE_WARP) mBatchass->getWarpsRef()->mouseDown(event);
	//if (mParameterBag->mMode == mParameterBag->MODE_AUDIO) mAudio->mouseDown(event);

}

void ReymentaMixnmapApp::mouseDrag(MouseEvent event)
{
	mMouse.x = (float)event.getPos().x;
	mMouse.y = (float)event.getPos().y;
	if (mParameterBag->mMode == mParameterBag->MODE_WARP) mBatchass->getWarpsRef()->mouseDrag(event);
	//if (mParameterBag->mMode == mParameterBag->MODE_AUDIO) mAudio->mouseDrag(event);

}
void ReymentaMixnmapApp::mouseUp(MouseEvent event)
{
	if (mParameterBag->mMode == mParameterBag->MODE_WARP) mBatchass->getWarpsRef()->mouseUp(event);
	//if (mParameterBag->mMode == mParameterBag->MODE_AUDIO) mAudio->mouseUp(event);
}
void ReymentaMixnmapApp::keyUp(KeyEvent event)
{
	if (mParameterBag->mMode == mParameterBag->MODE_WARP) mBatchass->getWarpsRef()->keyUp(event);

}
void ReymentaMixnmapApp::keyDown(KeyEvent event)
{

	int textureIndex = mParameterBag->iChannels[mParameterBag->selectedChannel];
	int keyCode = event.getCode();
	bool handled = false;
	mBatchass->getWarpsRef()->keyDown(event);
	if (!handled && (keyCode > 255 && keyCode < 265))
	{
		handled = true;
		mParameterBag->selectedChannel = keyCode - 256;
	}
	if (!handled)
	{
		switch (keyCode)
		{
		case ci::app::KeyEvent::KEY_c:
			mBatchass->createWarp();
			break;
		case ci::app::KeyEvent::KEY_s:
			if (event.isControlDown())
			{
				// save warp settings
				mBatchass->getWarpsRef()->save("warps2.xml");
				// save params
				mParameterBag->save();
			}
			break;
		case ci::app::KeyEvent::KEY_r:
			mParameterBag->controlValues[1] += 0.2;
			if (mParameterBag->controlValues[1] > 0.9) mParameterBag->controlValues[1] = 0.0;
			break;
		case ci::app::KeyEvent::KEY_g:
			mParameterBag->controlValues[2] += 0.2;
			if (mParameterBag->controlValues[2] > 0.9) mParameterBag->controlValues[2] = 0.0;
			break;
		case ci::app::KeyEvent::KEY_b:
			mParameterBag->controlValues[3] += 0.2;
			if (mParameterBag->controlValues[3] > 0.9) mParameterBag->controlValues[3] = 0.0;
			break;
		case ci::app::KeyEvent::KEY_p:
			mParameterBag->mPreviewEnabled = !mParameterBag->mPreviewEnabled;
			break;
		case ci::app::KeyEvent::KEY_v:
			mParameterBag->controlValues[48] = !mParameterBag->controlValues[48];
			break;
		case ci::app::KeyEvent::KEY_f:
			break;
		case ci::app::KeyEvent::KEY_h:
			if (mParameterBag->mCursorVisible)
			{
				mParameterBag->mShowUI = true;
				hideCursor();
			}
			else
			{
				mParameterBag->mShowUI = false;
				showCursor();
			}
			mParameterBag->mCursorVisible = !mParameterBag->mCursorVisible;
			break;
		case ci::app::KeyEvent::KEY_ESCAPE:
			mParameterBag->save();
			ui::Shutdown();
			mBatchass->shutdown();

			quit();
			break;
		case KeyEvent::KEY_SPACE:
			//random();
			mBatchass->getShadersRef()->random();
			break;
		case ci::app::KeyEvent::KEY_0:
		case 256:
			mParameterBag->selectedChannel = 0;
			break;
		case ci::app::KeyEvent::KEY_1:
		case 257:
			mParameterBag->selectedChannel = 1;
			break;
		case ci::app::KeyEvent::KEY_2:
		case 258:
			mParameterBag->selectedChannel = 2;
			break;
		case ci::app::KeyEvent::KEY_3:
		case 259:
			mParameterBag->selectedChannel = 3;
			break;
		case ci::app::KeyEvent::KEY_4:
		case 260:
			mParameterBag->selectedChannel = 4;
			break;
		case ci::app::KeyEvent::KEY_5:
		case 261:
			mParameterBag->selectedChannel = 5;
			break;
		case ci::app::KeyEvent::KEY_6:
		case 262:
			mParameterBag->selectedChannel = 6;
			break;
		case ci::app::KeyEvent::KEY_7:
		case 263:
			mParameterBag->selectedChannel = 7;
			break;
		case ci::app::KeyEvent::KEY_8:
		case 264:
			mParameterBag->selectedChannel = 8;
			break;
		case ci::app::KeyEvent::KEY_PLUS:
		case 270:
			textureIndex++;
			mBatchass->assignTextureToChannel(textureIndex, mParameterBag->selectedChannel);
			break;
		case ci::app::KeyEvent::KEY_MINUS:
		case 269:
			textureIndex--;
			mBatchass->assignTextureToChannel(textureIndex, mParameterBag->selectedChannel);
			break;
		default:
			break;
		}
	}
}

void ReymentaMixnmapApp::resize()
{
	mBatchass->getWarpsRef()->resize();
	// Create/resize frame buffers (no multisampling)
	mBufferCurrent = gl::Fbo::create(getWindowWidth(), getWindowHeight());
	mBufferNext = gl::Fbo::create(getWindowWidth(), getWindowHeight());
}

void ReymentaMixnmapApp::fileDrop(FileDropEvent event)
{
	// Send all file requests to the loading thread.
	size_t count = event.getNumFiles();
	//for (size_t i = 0; i < count && mRequests->isNotFull(); ++i) {
		//mRequests->pushFront(event.getFile(i));
	for (size_t i = 0; i < count ; ++i) {
		mBatchass->getShadersRef()->addRequest(event.getFile(i));
	}
	// TO MIGRATE
	int index;
	string ext = "";
	// use the last of the dropped files
	const fs::path &mPath = event.getFile(event.getNumFiles() - 1);
	string mFile = mPath.string();
	int dotIndex = mFile.find_last_of(".");
	int slashIndex = mFile.find_last_of("\\");

	if (dotIndex != std::string::npos && dotIndex > slashIndex) ext = mFile.substr(mFile.find_last_of(".") + 1);
	index = (int)(event.getX() / (margin + mParameterBag->mPreviewFboWidth + inBetween));// +1;
	//mBatchass->log(mFile + " dropped, currentSelectedIndex:" + toString(mParameterBag->currentSelectedIndex) + " x: " + toString(event.getX()) + " PreviewFboWidth: " + toString(mParameterBag->mPreviewFboWidth));

	if (ext == "wav" || ext == "mp3")
	{
		//mAudio->loadWaveFile(mFile);
	}
	else if (ext == "png" || ext == "jpg")
	{
		if (index < 1) index = 1;
		if (index > 3) index = 3;
		//mTextures->loadImageFile(mParameterBag->currentSelectedIndex, mFile);
		mBatchass->getTexturesRef()->loadImageFile(index, mFile);
	}
	else if (ext == "glsl")
	{
		if (index < 4) index = 4;
		int rtn = mBatchass->getShadersRef()->loadPixelFragmentShaderAtIndex(mFile, index);
		if (rtn > -1 && rtn < mBatchass->getShadersRef()->getCount())
		{
			mParameterBag->controlValues[22] = 1.0f;
			// TODO  send content via websockets
			/*fs::path fr = mFile;
			string name = "unknown";
			if (mFile.find_last_of("\\") != std::string::npos) name = mFile.substr(mFile.find_last_of("\\") + 1);
			if (fs::exists(fr))
			{

			std::string fs = loadString(loadFile(mFile));
			if (mParameterBag->mOSCEnabled) mOSC->sendOSCStringMessage("/fs", 0, fs, name);
			}*/
			// save thumb
			timeline().apply(&mTimer, 1.0f, 1.0f).finishFn([&]{ saveThumb(); });
		}
	}
	else if (ext == "mov" || ext == "mp4")
	{
		/*
		if (index < 1) index = 1;
		if (index > 3) index = 3;
		mBatchass->getTexturesRef()->loadMovieFile(index, mFile);*/
	}
	else if (ext == "fs")
	{
		//mShaders->incrementPreviewIndex();
		mBatchass->getShadersRef()->loadFragmentShader(mPath);
	}
	else if (ext == "xml")
	{
		mBatchass->getWarpsRef()->loadWarps(mFile);
	}
	else if (ext == "patchjson")
	{
		// try loading patch
		try
		{
			JsonTree patchjson;
			try
			{
				patchjson = JsonTree(loadFile(mFile));
				mParameterBag->mCurrentFilePath = mFile;
			}
			catch (cinder::JsonTree::Exception exception)
			{
				CI_LOG_V("patchjsonparser exception " + mFile + ": " + exception.what());

			}
			//Assets
			int i = 1; // 0 is audio
			JsonTree jsons = patchjson.getChild("assets");
			for (JsonTree::ConstIter jsonElement = jsons.begin(); jsonElement != jsons.end(); ++jsonElement)
			{
				string jsonFileName = jsonElement->getChild("filename").getValue<string>();
				int channel = jsonElement->getChild("channel").getValue<int>();
				if (channel < mBatchass->getTexturesRef()->getTextureCount())
				{
					CI_LOG_V("asset filename: " + jsonFileName);
					mBatchass->getTexturesRef()->setTexture(channel, jsonFileName);
				}
				i++;
			}

		}
		catch (...)
		{
			CI_LOG_V("patchjson parsing error: " + mFile);
		}
	}
	else if (ext == "txt")
	{
		// try loading shader parts
		if (mBatchass->getShadersRef()->loadTextFile(mFile))
		{

		}
	}
	else if (ext == "")
	{
		// try loading image sequence from dir
		if (index < 1) index = 1;
		if (index > 3) index = 3;
		mBatchass->getTexturesRef()->createFromDir(mFile + "/", index);
		// or create thumbs from shaders
		mBatchass->getShadersRef()->createThumbsFromDir(mFile + "/");
	}
	/*if (!loaded && ext == "frag")
	{

	//mShaders->incrementPreviewIndex();

	if (mShaders->loadPixelFrag(mFile))
	{
	mParameterBag->controlValues[22] = 1.0f;
	timeline().apply(&mTimer, 1.0f, 1.0f).finishFn([&]{ save(); });
	}
	if (mCodeEditor) mCodeEditor->fileDrop(event);
	}*/
	mParameterBag->isUIDirty = true;
}
// From imgui by Omar Cornut
void ReymentaMixnmapApp::ShowAppConsole(bool* opened)
{
	mConsole->Run("Console", opened);
}
void ReymentaMixnmapApp::saveThumb()
{
	/* TODO
	string filename;
	try
	{
	filename = mBatchass->getShadersRef()->getFragFileName() + ".jpg";
	writeImage(getAssetPath("") / "thumbs" / filename, mBatchass->getTexturesRef()->getFboTexture(mParameterBag->mCurrentPreviewFboIndex));
	mBatchass->log("saved:" + filename);
	}
	catch (const std::exception &e)
	{
	mBatchass->log("unable to save:" + filename + string(e.what()));
	}*/
}
void ReymentaMixnmapApp::setUniforms()
{
	auto shader = gl::context()->getGlslProg();
	if (!shader)
		return;

	// Calculate shader parameters.
	vec3  iResolution(vec2(getWindowSize()), 1);
	mParameterBag->iChannelTime[0] = (float)getElapsedSeconds();
	mParameterBag->iChannelTime[1] = (float)getElapsedSeconds() - 1;
	mParameterBag->iChannelTime[2] = (float)getElapsedSeconds() - 2;
	mParameterBag->iChannelTime[3] = (float)getElapsedSeconds() - 3;
	//
	if (mParameterBag->mUseTimeWithTempo)
	{
		mParameterBag->iGlobalTime = mParameterBag->iTempoTime*mParameterBag->iTimeFactor;
	}
	else
	{
		mParameterBag->iGlobalTime = (float)getElapsedSeconds();
	}
	mParameterBag->iGlobalTime *= mParameterBag->iSpeedMultiplier;

	vec3  iChannelResolution0 = mChannel0 ? vec3(mChannel0->getSize(), 1) : vec3(1);
	vec3  iChannelResolution1 = mChannel1 ? vec3(mChannel1->getSize(), 1) : vec3(1);
	vec3  iChannelResolution2 = mChannel2 ? vec3(mChannel2->getSize(), 1) : vec3(1);
	vec3  iChannelResolution3 = mChannel3 ? vec3(mChannel3->getSize(), 1) : vec3(1);

	time_t now = time(0);
	tm*    t = gmtime(&now);
	vec4   iDate(float(t->tm_year + 1900),
		float(t->tm_mon + 1),
		float(t->tm_mday),
		float(t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec));

	// Set shader uniforms.
	shader->uniform("iResolution", iResolution);
	shader->uniform("iGlobalTime", mParameterBag->iGlobalTime);
	shader->uniform("iChannelTime[0]", mParameterBag->iChannelTime[0]);
	shader->uniform("iChannelTime[1]", mParameterBag->iChannelTime[1]);
	shader->uniform("iChannelTime[2]", mParameterBag->iChannelTime[2]);
	shader->uniform("iChannelTime[3]", mParameterBag->iChannelTime[3]);
	shader->uniform("iChannelResolution[0]", iChannelResolution0);
	shader->uniform("iChannelResolution[1]", iChannelResolution1);
	shader->uniform("iChannelResolution[2]", iChannelResolution2);
	shader->uniform("iChannelResolution[3]", iChannelResolution3);
	shader->uniform("iMouse", mMouse);
	shader->uniform("iChannel0", 0);
	shader->uniform("iChannel1", 1);
	shader->uniform("iChannel2", 2);
	shader->uniform("iChannel3", 3);
	shader->uniform("iDate", iDate);
}

#pragma warning(pop) // _CRT_SECURE_NO_WARNINGS

CINDER_APP(ReymentaMixnmapApp, RendererGl, &ReymentaMixnmapApp::prepare)
