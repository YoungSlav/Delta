#pragma once

#include "stdafx.h"
#include "Object.h"

namespace Delta
{


class Asset : public Object
{
public:
	template <typename... Args>
	Asset(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	bool Load();

protected:
	virtual void OnDestroy() override;
	virtual bool Load_Internal();

	virtual void Cleanup();

private:
	
private:

	bool bLoaded = false;
};

}