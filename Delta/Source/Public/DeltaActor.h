#pragma once
#include "stdafx.h"
#include "DeltaObject.h"
#include "DeltaMath.h"
#include "IDeltaTransformable.h"
#include "DeltaComponent.h"

class DeltaActor : public DeltaObject, public IDeltaTransformable
{
public:
	template <typename... Args>
	DeltaActor(Args&&... args) :
		DeltaObject(std::forward<Args>(args)...),
		IDeltaTransformable()
	{}

	void SetVisisble(bool bNewVisible, bool bPropagateToChildren = false);
	bool IsVisisble() const { return bVisible; }

	const std::vector<std::shared_ptr<DeltaComponent>>& GetComponents() const { return Components; }

protected:
	virtual void OnChildObjectAdded(std::shared_ptr<DeltaObject> _OwnedObject) override;
	virtual void OnChildObjectRemoved(std::shared_ptr<DeltaObject> _OwnedObject) override;
	virtual void OnDestroy() override;

protected:

	std::vector<std::shared_ptr<DeltaComponent>> Components;

	bool bVisible = true;
};
