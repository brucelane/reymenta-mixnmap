#include "OSCWrapper.h"

using namespace Reymenta;

OSC::OSC( ParameterBagRef aParameterBag )
{
	mParameterBag = aParameterBag;
	// OSC sender
	//mOSCSender.setup(mParameterBag->mOSCDestinationHost, mParameterBag->mOSCDestinationPort);
	// OSC receiver
	mOSCReceiver.setup(mParameterBag->mOSCReceiverPort);
}

OSCRef OSC::create( ParameterBagRef aParameterBag )
{
	return shared_ptr<OSC>( new OSC( aParameterBag ) );
}

void OSC::update()
{
	// osc
	while( mOSCReceiver.hasWaitingMessages() ) 
	{
		osc::Message message;
		mOSCReceiver.getNextMessage( &message );
		int arg0 = 0;
		int arg1 = 0;
		int arg2 = 0;
		int arg3 = 0;
		int skeletonIndex = 0;
		int jointIndex = 0;
		string oscAddress = message.getAddress();
		string oscArg0 = "";
		string oscArg1 = "";
		string oscArg2 = "";
		string oscArg3 = "";
		int numArgs = message.getNumArgs();
		if(oscAddress == "/midi")
		{
			for (int i = 0; i < message.getNumArgs(); i++) {
				cout << "-- Argument " << i << std::endl;
				cout << "---- type: " << message.getArgTypeName(i) << std::endl;
				if (message.getArgType(i) == osc::TYPE_INT32) {
					try {
						//cout << "------ int value: "<< message.getArgAsInt32(i) << std::endl;
						if (i == 0)
						{
							arg0 = message.getArgAsInt32(i);
							oscArg0 = toString(message.getArgAsInt32(i));
						}
						if (i == 1)
						{
							arg1 = message.getArgAsInt32(i);
							oscArg1 = toString(message.getArgAsInt32(i));
						}
						if (i == 2)
						{
							arg2 = message.getArgAsInt32(i);
							oscArg2 = toString(message.getArgAsInt32(i));
						}
						if (i == 3)
						{
							arg3 = message.getArgAsInt32(i);
							oscArg3 = toString(message.getArgAsInt32(i));
						}
					}
					catch (...) {
						cout << "Exception reading argument as int32" << std::endl;
					}
				}
			}
			if (arg0 < 0) arg0 = 0;
			if (arg1 < 0) arg1 = 0;
			if (arg0 > 4096) arg0 = 4096;
			if (arg1 > 4096) arg1 = 4096;
			float normalizedValue = lmap<float>(arg1, 0.0, 127.0, 0.0, 1.0);
			switch (arg0)
			{
			case 15:
				// crossfade
				mParameterBag->controlValues[arg0] = normalizedValue;
				break;
			}

		}
		
		string oscString = "osc from:" + message.getRemoteIp() + " adr:" + oscAddress + " 0: " + oscArg0 + " 1: " + oscArg1;
		mParameterBag->OSCMsg = oscString;
	}	
}
