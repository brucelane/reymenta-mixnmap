/**
* \file AudioWrapper.cpp
* \author Bruce LANE
* \date 24 november 2014
*
* Manages the Audio.
*
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

	//! create the audio texture
	unsigned char				dTexture[1024];
	for (int i = 0; i < 1024; ++i) dTexture[i] = (unsigned char)(Rand::randUint() & 0xFF);
	// store it as a 512x2 texture in the first texture
	gl::TextureRef img = gl::Texture::create(dTexture, GL_LUMINANCE16I_EXT, 512, 2);

	audioTextureIndex = mTextures->createTexture("Audio", 512, 2, img);
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
		mTextures->setAudioTexture(audioTextureIndex, signal);
	}
}
