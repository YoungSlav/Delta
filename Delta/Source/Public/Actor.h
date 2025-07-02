#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Math.h"
#include "ITransformable.h"
#include "ActorComponent.h"

namespace Delta
{
class Actor : public Object, public ITransformable
{
public:
	template <typename... Args>
	Actor(Args&&... args) :
		Object(std::forward<Args>(args)...),
		ITransformable()
	{}

	void SetVisisble(bool bNewVisible, bool bPropagateToChildren = false);
	bool IsVisisble() const { return bVisible; }

	const std::vector<std::shared_ptr<ActorComponent>>& GetComponents() const { return Components; }

protected:
	virtual void OnChildObjectAdded(std::shared_ptr<Object> _OwnedObject) override;
	virtual void OnChildObjectRemoved(std::shared_ptr<Object> _OwnedObject) override;
	virtual void OnDestroy() override;

protected:

	std::vector<std::shared_ptr<ActorComponent>> Components;

	bool bVisible = true;
};

}