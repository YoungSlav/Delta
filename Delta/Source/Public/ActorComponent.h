#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Math.h"
#include "ITransformable.h"



namespace Delta
{
class ActorComponent : public Object, public ITransformable
{

public:
	template <typename... Args>
	ActorComponent(Args&&... args) :
		Object(std::forward<Args>(args)...),
		ITransformable()
	{
		setParent(GetOwningActor());
	}


	std::shared_ptr<class Actor> GetOwningActor() const;
protected:
	virtual bool initialize_Internal() override;
};
}