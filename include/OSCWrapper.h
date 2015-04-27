#pragma once

#include "cinder/app/App.h"
#include "OscListener.h"
#include "OSCSender.h"
#include "Resources.h"
#include "ParameterBag.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta 
{

	typedef std::shared_ptr<class OSC> OSCRef;

	class OSC 
	{

	public:
		OSC( ParameterBagRef aParameterBag );
		static	OSCRef				create( ParameterBagRef aParameterBag );
		void						sendOSCMessage(string controlType, int arg0, int arg1, int arg2, int arg3);
		void						sendOSCFloatMessage(string controlType, int controlName, float x, float y, float z);
		void						sendOSCStringMessage(string controlType, int index, string s);
		void						setupReceiver();
		void						update();
	private:
		// parameters
		ParameterBagRef				mParameterBag;
		osc::Listener 				mOSCReceiver;
		osc::Sender					mOSCSender;
	};
}