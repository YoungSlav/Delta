#pragma once

#include "stdafx.h"
#include "Object.h"

namespace Delta
{

enum EAssetLoadingState : int32
{
	INVALID,
	ASYNC_LOADING,
	LOADED
};

class Asset : public Object
{
public:
	template <typename... Args>
	Asset(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	EAssetLoadingState load();
	void cleanup();

	EAssetLoadingState getLoadingState() const { return state; }
	inline bool isReady() const { return state == EAssetLoadingState::LOADED; }

protected:
	virtual void onDestroy() override;
	virtual EAssetLoadingState load_Internal() = 0;
	virtual void cleanup_Internal() = 0;

private:
	
private:

	EAssetLoadingState state = EAssetLoadingState::INVALID;
};

}