#pragma once

#include "cinder/app/AppNative.h"
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
		static	OSCRef create( ParameterBagRef aParameterBag );

		void	update();
	private:
		// parameters
		ParameterBagRef mParameterBag;
		osc::Listener 				mOSCReceiver;
		osc::Sender					mOSCSender;
	};
}