#include "Spaghetti.h"

using namespace Reymenta;

Spaghetti::Spaghetti(ParameterBagRef aParameterBag, TexturesRef aTextures)
{
	mParameterBag = aParameterBag;
	mTextures = aTextures;
	// instanciate the logger class
	log = Logger::create("Spaghetti.txt");
	log->logTimedString("Spaghetti constructor");

	// bezier lines
	currentPath = 0;
	mPath.push_back(new Path2d());

	log->logTimedString("Spaghetti constructor end");

}
void Spaghetti::drawPath()
{
	// draw bezier
	/*if (mPath[currentPath]->calcLength() > 2)
	{
		//mPath[currentPath]->clear();
		mPath.push_back(new Path2d());
		currentPath++;
	}
	if (mPath[currentPath]->empty()) {
		mPath[currentPath]->moveTo(mMousePos);
		mTrackedPoint = 0;
	}
	else
		mPath[currentPath]->lineTo(mMousePos);
	*/
}
void Spaghetti::mouseDown(MouseEvent event)
{
	mMousePos = vec2(event.getX(), event.getY());
}
void Spaghetti::mouseUp(MouseEvent event)
{
	mTrackedPoint = -1;
}
void Spaghetti::mouseDrag(MouseEvent event)
{

	if (mPath[currentPath]->calcLength() > 1) {

		if (mTrackedPoint >= 0) {
			mPath[currentPath]->setPoint(mTrackedPoint, event.getPos());
		}
		else 
		{ 
			// first bit of dragging, so switch our line to a cubic or a quad if Shift is down
			// we want to preserve the end of our current line, because it will still be the end of our curve
			vec2 endPt = mPath[currentPath]->getPoint(mPath[currentPath]->getNumPoints() - 1);
			// and now we'll delete that line and replace it with a curve
			mPath[currentPath]->removeSegment(mPath[currentPath]->getNumSegments() - 1);

			Path2d::SegmentType prevType = (mPath[currentPath]->getNumSegments() == 0) ? Path2d::MOVETO : mPath[currentPath]->getSegmentType(mPath[currentPath]->getNumSegments() - 1);

			if (event.isShiftDown() || prevType == Path2d::MOVETO) { // add a quadratic curve segment
				mPath[currentPath]->quadTo(event.getPos(), endPt);
			}

			// our second-to-last point is the tangent next to the end, and we'll track that
			mTrackedPoint = mPath[currentPath]->getNumPoints() - 2;
		}

	}
}
void Spaghetti::draw()
{
	gl::color(Color(1, 1, 0));
	for (auto & path : mPath)
	{
		for (size_t p = 0; p < path->getNumPoints(); ++p)
			gl::drawSolidCircle(path->getPoint(p), 2.5f);

		// draw the curve itself
		gl::color(Color(1.0f, 0.5f, 0.25f));
		gl::draw(*path);

		if (path->getNumSegments() > 1) {
			// draw some tangents
			gl::color(Color(0.2f, 0.9f, 0.2f));
			for (float t = 0; t < 1; t += 0.2f)
				gl::drawLine(path->getPosition(t), path->getPosition(t) + normalize(path->getTangent(t)) * 80.0f);

			// draw circles at 1/4, 2/4 and 3/4 the length
			gl::color(ColorA(0.2f, 0.9f, 0.9f, 0.5f));
			for (float t = 0.25f; t < 1.0f; t += 0.25f)
				gl::drawSolidCircle(path->getPosition(path->calcNormalizedTime(t)), 5.0f);
		}

	}
}
