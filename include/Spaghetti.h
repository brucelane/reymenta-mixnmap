#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/App.h"

#include "Logger.h"

// textures
#include "Textures.h"
// Parameters
#include "ParameterBag.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Reymenta
{
	// stores the pointer to the Spaghetti instance
	typedef std::shared_ptr<class Spaghetti> SpaghettiRef;

	class Spaghetti {
	public:
		Spaghetti(ParameterBagRef aParameterBag, TexturesRef aTexturesRef);
		static SpaghettiRef	create(ParameterBagRef aParameterBag, TexturesRef aTexturesRef)
		{
			return shared_ptr<Spaghetti>(new Spaghetti(aParameterBag, aTexturesRef));
		}

		void						mouseMove(MouseEvent event);
		void						mouseDown(MouseEvent event);
		void						mouseDrag(MouseEvent event);
		void						mouseUp(MouseEvent event);
		void						keyDown(KeyEvent event);
		void						keyUp(KeyEvent event);
		void						draw();

		void						drawPath();
	private:
		// Logger
		LoggerRef					log;
		// Parameters
		ParameterBagRef				mParameterBag;
		// Textures
		TexturesRef							mTextures;
		// bezier lines for nodes
		vector<Path2d*>						mPath;
		int									mTrackedPoint;
		int									currentPath;
		// mouse position
		vec2								mMousePos;

	};
}