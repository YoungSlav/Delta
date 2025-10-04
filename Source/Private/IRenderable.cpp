#include "IRenderable.h"

using namespace Delta;


void IRenderable::setVisisble(bool bNewVisible)
{
	bVisible = bNewVisible;
}
bool IRenderable::isVisisble() const { return bVisible; }