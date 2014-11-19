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
#include "AudioWrapper.h"

using namespace Reymenta;

AudioWrapper::AudioWrapper(ParameterBagRef aParameterBag, TexturesRef aTexturesRef)
{
	mParameterBag = aParameterBag;
	mTextures = aTexturesRef;
	// instanciate the logger class
	log = Logger::create("AudioLog.txt");
	log->logTimedString("Audio constructor");

	// linein
	auto ctx = audio::Context::master();
	mLineIn = ctx->createInputDeviceNode();

	auto scopeLineInFmt = audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorLineInSpectralNode = ctx->makeNode(new audio::MonitorSpectralNode(scopeLineInFmt));

	mLineIn >> mMonitorLineInSpectralNode;

	mLineIn->enable();

	// wave
	// TODO: it is pretty surprising when you recreate mScope here without checking if there has already been one added.
	//	- user will no longer see the old mScope, but the context still owns a reference to it, so another gets added each time we call this method.
	auto scopeWaveFmt = audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorWaveSpectralNode = ctx->makeNode(new audio::MonitorSpectralNode(scopeWaveFmt));

	ctx->enable();
}

AudioWrapperRef AudioWrapper::create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef)
{
	return shared_ptr<AudioWrapper>(new AudioWrapper(aParameterBag, aTexturesRef));
}
void AudioWrapper::loadWaveFile(string aFilePath)
{
	try
	{
		if (!fs::exists(aFilePath))
		{
			log->logTimedString("file not found: " + aFilePath);

		}
		else
		{
			log->logTimedString("found file: " + aFilePath);
			auto ctx = audio::master();
			mSourceFile = audio::load(loadFile(aFilePath), audio::master()->getSampleRate());
			mSamplePlayerNode = ctx->makeNode(new audio::FilePlayerNode(mSourceFile, false));
			mSamplePlayerNode->setLoopEnabled(false);
			mSamplePlayerNode >> mMonitorWaveSpectralNode >> ctx->getOutput();
			mSamplePlayerNode->enable();
			//ctx->enable();

			// or connect in series (it is added to the Context's 'auto pulled list')
			//mSamplePlayerNode >> ctx->getOutput();
			mSamplePlayerNode->seek(0);

			auto filePlayer = dynamic_pointer_cast<audio::FilePlayerNode>(mSamplePlayerNode);
			CI_ASSERT_MSG(filePlayer, "expected sample player to be either BufferPlayerNode or FilePlayerNode");

			filePlayer->setSourceFile(mSourceFile);

			//audio::master()->printGraph();

			mSamplePlayerNode->start();
			mParameterBag->mUseLineIn = false;
		}
	}
	catch (...)
	{
		log->logTimedString("loadWaveFile error: " + aFilePath);

	}
}

void AudioWrapper::update()
{
	if (mParameterBag->mUseLineIn || !mSamplePlayerNode)
	{
		mMagSpectrum = mMonitorLineInSpectralNode->getMagSpectrum();
	}
	else
	{
		mMagSpectrum = mMonitorWaveSpectralNode->getMagSpectrum();
	}
	if (mMagSpectrum.empty())
		return;

	unsigned char signal[kBands];
	mParameterBag->maxVolume = 0.0;
	size_t mDataSize = mMagSpectrum.size();
	if (mDataSize > 0)
	{
		float mv;
		float db;
		float maxdb = 0.0f;
		for (size_t i = 0; i < mDataSize; i++) {
			float f = mMagSpectrum[i];
			db = audio::linearToDecibel(f);
			f = db * mParameterBag->mAudioMultFactor;
			if (f > mParameterBag->maxVolume)
			{
				mParameterBag->maxVolume = f; mv = f;
			}
			mParameterBag->mData[i] = f;
			int ger = f;
			signal[i] = static_cast<unsigned char>(ger);

			if (db > maxdb) maxdb = db;

			switch (i)
			{
			case 11:
				mParameterBag->iFreqs[0] = f;
				break;
			case 13:
				mParameterBag->iFreqs[1] = f;
				break;
			case 15:
				mParameterBag->iFreqs[2] = f;
				break;
			case 18:
				mParameterBag->iFreqs[3] = f;
				break;
			default:
				break;
			}

		}
		// store it as a 512x2 texture in UPDATE only!!
		mTextures->setAudioTexture(signal);
	}
}
