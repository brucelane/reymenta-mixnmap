#pragma once

#include "cinder/app/AppNative.h"
// Parameters
#include "ParameterBag.h"
// textures
#include "Textures.h"
// log
#include "Logger.h"
// audio
#include "cinder/audio/Context.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Utilities.h"
#include "cinder/audio/Source.h"
#include "cinder/audio/Target.h"
#include "cinder/audio/dsp/Converter.h"
#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/audio/SampleRecorderNode.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/MonitorNode.h"
//#include "AudioDrawUtils.h"
#include "cinder/gl/Vbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta 
{

	typedef std::shared_ptr<class AudioWrapper> AudioWrapperRef;

	class AudioWrapper 
	{

	public:
		AudioWrapper( ParameterBagRef aParameterBag, TexturesRef aTexturesRef );
		static AudioWrapperRef create( ParameterBagRef aParameterBag, TexturesRef aTexturesRef );
		void update();
		void loadWaveFile(string aFilePath);

	private:

		// parameters
		ParameterBagRef					mParameterBag;
		// Textures
		TexturesRef						mTextures;
		// Logger
		LoggerRef						log;

		// audio
		audio::InputDeviceNodeRef		mLineIn;
		audio::MonitorSpectralNodeRef	mMonitorLineInSpectralNode;
		audio::MonitorSpectralNodeRef	mMonitorWaveSpectralNode;

		vector<float>					mMagSpectrum;

		audio::SamplePlayerNodeRef		mSamplePlayerNode;
		audio::SourceFileRef			mSourceFile;
		// Paul Houx
		// width and height of our mesh
		static const int kWidth = 512;
		static const int kHeight = 512;

		// number of frequency bands of our spectrum
		static const int kBands = 1024;
		static const int kHistory = 128;

		Channel32f			mChannelLeft;
		Channel32f			mChannelRight;

		bool				mIsMouseDown;
		bool				mIsAudioPlaying;

	};
}