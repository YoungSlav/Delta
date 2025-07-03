#pragma once

#include "stdafx.h"
#include "Object.h"

namespace Delta
{

enum EAssetLoadingState : int32
{
	Invalid,
	AsyncLoading,
	Loaded
};

class Asset : public Object
{
public:
	template <typename... Args>
	Asset(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	EAssetLoadingState Load();
	void Cleanup();

	EAssetLoadingState GetLoadingState() const { return State; }
	inline bool IsReady() const { return State == EAssetLoadingState::Loaded; }

protected:
	virtual void OnDestroy() override;
	virtual EAssetLoadingState Load_Internal() = 0;
	virtual void Cleanup_Internal() = 0;

private:
	
private:

	EAssetLoadingState State = EAssetLoadingState::Invalid;
};

}