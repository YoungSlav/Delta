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

protected:
	

private:
	bool bVisible = true;
};

}