#pragma once
#include "stdafx.h"

namespace Delta
{

class IRenderable
{

public:
	IRenderable() = default;

	virtual ~IRenderable() = default;

	void setVisisble(bool bNewVisible);
	virtual bool isVisisble() const;

	virtual void render(const struct CameraInfo& cameraInfo) = 0;

protected:
	

private:
	bool bVisible = true;
};

}