#include "SpoutWrapper.h"

using namespace Reymenta;

SpoutWrapper::SpoutWrapper(ParameterBagRef aParameterBag, TexturesRef aTextures)
{
	mParameterBag = aParameterBag;
	mTextures = aTextures;
	// instanciate the logger class
	log = Logger::create("SpoutWrapperLog.txt");
	log->logTimedString("SpoutWrapper constructor");

	bInitialized = false;
	nReceivers = 0;

	log->logTimedString("SpoutWrapper constructor end");

}

void SpoutWrapper::update()
{
	bool found = false;
	nSenders = mSpoutReceivers[0].GetSenderCount();

	if (nSenders != nReceivers && nSenders > 0)
	{
		//! the name will be filled when the receiver connects to a sender
		mNewSenderName[0] = NULL;
		log->logTimedString("new sender found or sender deleted");

		nReceivers = 0;
		//! loop to find existing sender with that name
		for (int i = 0; i < nSenders; i++)
		{
			mSpoutReceivers[0].GetSenderName(i, &mNewSenderName[0], MaxSize);
			memcpy(mTextures->getSenderName(i), mNewSenderName, strlen(mNewSenderName) + 1);
			// fix for old beta of Spout, should not be set to "true"
			mSpoutReceivers[i].SetDX9(mParameterBag->mUseDX9);
			log->logTimedString("DX9:" + toString(mParameterBag->mUseDX9));

			if (mSpoutReceivers[i].CreateReceiver(mTextures->getSenderName(i), mNewWidth, mNewHeight))
			{
				bInitialized = true;
				mTextures->setSenderTextureSize(i, mNewWidth, mNewHeight);

				log->logTimedString("create receiver name:");
				log->logTimedString(mTextures->getSenderName(i));
				nReceivers++;
				log->logTimedString("new receiver count:");
				log->logTimedString(toString(nReceivers));
			}
		}
	}
}

void SpoutWrapper::draw()
{
	unsigned int width, height;
	int actualReceivers = 0;
	gl::setMatricesWindow(getWindowSize());

	//
	// Try to receive the texture at the current size 
	//
	// NOTE : if ReceiveTexture is called with a framebuffer object bound, that binding will be lost
	// and has to be restored afterwards because Spout uses an fbo for intermediate rendering
	if (bInitialized)
	{
		for (int i = 0; i < nReceivers; i++)
		{
			if (mSpoutReceivers[i].ReceiveTexture(mTextures->getSenderName(i), width, height, mTextures->getSenderTexture(i)->getId(), mTextures->getSenderTexture(i)->getTarget()))
			{
				mTextures->setSenderTextureSize(i, width, height);
				//senders[i].width = width;
				//senders[i].height = height;
				actualReceivers++;
			}
			else
			{
				mSpoutReceivers[i].ReleaseReceiver();
			}
		}
	}
	nReceivers = actualReceivers;
}

void SpoutWrapper::shutdown()
{
	for (int i = 0; i < nReceivers; i++)
	{
		mSpoutReceivers[i].ReleaseReceiver();
	}
}
