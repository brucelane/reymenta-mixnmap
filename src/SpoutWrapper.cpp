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
	mNewSenderName[0] = NULL;
	for (int i = 0; i < MAX; i++)
	{
		memcpy(&senders[i].SenderName[0], mNewSenderName, strlen(mNewSenderName) + 1);
		senders[i].width = 320;
		senders[i].height = 240;
		//senders[i].active = false;
	}
	/*char SenderName[256];
	mSpoutSharedMemoryReceiver.SetMemoryShareMode(true);
	mSpoutSharedMemoryReceiver.GetActiveSender(SenderName);
	unsigned int width = 0;
	unsigned int height = 0;
	bool bMemoryMode;
	HANDLE hShareHandle;
	DWORD dwFormat;
	//mSpoutSharedMemoryReceiver.GetSenderInfo(SenderName, width, height, hShareHandle, dwFormat);
	mSpoutSharedMemoryReceiver.GetImageSize(SenderName, width, height, bMemoryMode);*/
	log->logTimedString("SpoutWrapper constructor end");

}

void SpoutWrapper::update()
{
	unsigned int width, height;
	bool found = false;
	nSenders = mSpoutReceivers[0].GetSenderCount();

	//if (nSenders > nReceivers)
	if (nSenders != nReceivers && nSenders > 0)
	{
		mNewSenderName[0] = NULL;// the name will be filled when the receiver connects to a sender
		log->logTimedString("new sender found or sender deleted");

		//int index = nReceivers; // or based on GetSenderCount
		nReceivers = 0;
		// loop to find existing sender with that name
		for (int i = 0; i < nSenders; i++)
		{
			mSpoutReceivers[0].GetSenderName(i, &mNewSenderName[0], MaxSize);
			memcpy(&senders[i].SenderName[0], mNewSenderName, strlen(mNewSenderName) + 1);
			//fix 
			mSpoutReceivers[i].SetDX9(mParameterBag->mUseDX9);
			log->logTimedString("DX9:" + toString(mParameterBag->mUseDX9));

			if (mSpoutReceivers[i].CreateReceiver(&senders[i].SenderName[0], mNewWidth, mNewHeight))//, true)) // true to find the active sender
			{
				bInitialized = true;

				// test for texture share compatibility
				// bMemoryMode informs us whether Spout initialized for texture share or memory share
				mParameterBag->mMemoryMode = mSpoutReceivers[i].GetMemoryShareMode();
				log->logTimedString("GetMemoryShareMode:" + toString(mParameterBag->mMemoryMode));
				mSpoutReceivers[i].SetMemoryShareMode(mParameterBag->mMemoryMode);

				senders[i].width = mNewWidth;
				senders[i].height = mNewHeight;
				//mSenderNames.push_back(&senders[index].SenderName[0]);
				log->logTimedString("create receiver name:");
				log->logTimedString(&senders[i].SenderName[0]);
				nReceivers++;
				log->logTimedString("new receiver count:");
				log->logTimedString(toString(nReceivers));
			}
			/*if (!found)
			{
			// still searching
			mSpoutReceivers[0].GetSenderName(i, &mNewSenderName[0], MaxSize);
			int j = 0;

			// loop on the mSenderNames vector to find if the name already exists
			do
			{
			if (strcmp(senders[j].SenderName, &mNewSenderName[0]) != 0)
			{
			found = true;
			memcpy(&senders[nReceivers].SenderName[0], mNewSenderName, strlen(mNewSenderName) + 1);
			log->logTimedString("found:");
			log->logTimedString(&senders[nReceivers].SenderName[0]);
			}
			j++;

			} while (!found && j < MAX);// senders.size());

			}*/
		}
		/*if (!found)
		{
		if (mSpoutReceivers[index].CreateReceiver(&senders[nReceivers].SenderName[0], mNewWidth, mNewHeight))//, true)) // true to find the active sender
		{
		bInitialized = true;
		senders[index].width = mNewWidth;
		senders[index].height = mNewHeight;
		//mSenderNames.push_back(&senders[index].SenderName[0]);
		log->logTimedString("create receiver name:");
		log->logTimedString(&senders[index].SenderName[0]);
		nReceivers++;
		log->logTimedString("new receiver count:");
		log->logTimedString(toString(nReceivers));
		}
		}*/
	}
}

void SpoutWrapper::draw()
{
	unsigned int width, height;
	char txt[256];
	int currentActiveSender = 0;
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
			//if (mSpoutReceivers[i].ReceiveTexture(mSenderNames[i], width, height, sTextures[i].getId(), sTextures[i].getTarget()))
			if (mSpoutReceivers[i].ReceiveTexture(senders[i].SenderName, width, height, mTextures->getTexture(i).getId(), mTextures->getTexture(i).getTarget()))
			{
				mTextures->setTextureSize(i, width, height);
				senders[i].width = width;
				senders[i].height = height;
				//senders[i].active = true;
				//sTextures[i] = gl::Texture(width, height);
				mParameterBag->iChannels[currentActiveSender] = i;
				currentActiveSender++;
				actualReceivers++;
			}
			else
			{
				//senders[i].active = false;
				//mNewSenderName[0] = NULL;
				//memcpy(&senders[i].SenderName[0], mNewSenderName, strlen(mNewSenderName) + 1);
				mSpoutReceivers[i].ReleaseReceiver();
			}
		}
		mTextures->draw();
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
