#pragma once
#include "stdafx.h"
#include "DeltaObject.h"
#include "DeltaMath.h"
#include "IDeltaTransformable.h"

class DeltaComponent : public DeltaObject, public IDeltaTransformable
{

public:
	template <typename... Args>
	DeltaComponent(Args&&... args) :
		DeltaObject(std::forward<Args>(args)...),
		IDeltaTransformable()
	{
		SetParent(GetOwningActor());
	}


	std::shared_ptr<class DeltaActor> GetOwningActor() const;
protected:
	virtual bool Initialize_Internal() override;
};