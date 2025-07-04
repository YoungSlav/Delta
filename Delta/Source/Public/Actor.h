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

	void setVisisble(bool bNewVisible, bool bPropagateToChildren = false);
	bool isVisisble() const { return bVisible; }

	const std::vector<std::shared_ptr<ActorComponent>>& getComponents() const { return components; }

protected:
	virtual void onChildObjectAdded(std::shared_ptr<Object> _OwnedObject) override;
	virtual void onChildObjectRemoved(std::shared_ptr<Object> _OwnedObject) override;
	virtual void onDestroy() override;

protected:

	std::vector<std::shared_ptr<ActorComponent>> components;

	bool bVisible = true;
};

}