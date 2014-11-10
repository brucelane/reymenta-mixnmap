#pragma once

#include <string>
#include <vector>

#include "cinder/Cinder.h"
#include "cinder/app/AppNative.h"
#include "Logger.h"

// parameters
#include "ParameterBag.h"
// textures
#include "Textures.h"
// spout
#include "Spout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta
{
	// stores the pointer to the Textures instance
	typedef std::shared_ptr<class SpoutWrapper> SpoutWrapperRef;

	class SpoutWrapper {
	public:
		SpoutWrapper(ParameterBagRef aParameterBag, TexturesRef aTextures);
		static SpoutWrapperRef	create(ParameterBagRef aParameterBag, TexturesRef aTextures)
		{
			return shared_ptr<SpoutWrapper>(new SpoutWrapper(aParameterBag, aTextures));
		}
		void						update();
		void						draw();
		void						shutdown();
		char *						getSenderName(int index) { return senders[index].SenderName; };
		unsigned int				getSenderWidth(int index) { return senders[index].width; };
		unsigned int				getSenderHeight(int index) { return senders[index].height; };
		int							getSenderCount() { return nReceivers; };
	private:
		// Logger
		LoggerRef					log;

		// parameters
		ParameterBagRef				mParameterBag;
		// textures
		TexturesRef					mTextures;

		// -------- SPOUT -------------
		static const int			MAX = 8;
		SpoutReceiver mSpoutReceivers[MAX];				// Create a Spout receiver array
		bool bInitialized;								// true if a sender initializes OK
		unsigned int mNewWidth, mNewHeight;				// size of the texture being sent out
		char mNewSenderName[256];						// new sender name 

		int MaxSize = 256;
		int nSenders;
		int nReceivers;

		struct SENDER {
			char SenderName[256];
			unsigned int width, height;
		} senders[MAX];
	};
}