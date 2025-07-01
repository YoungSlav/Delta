#pragma once
#include "stdafx.h"
#include "DeltaTransform.h"

typedef MulticastDelegate<const class IDeltaTransformable*> OnTransformUpdatedSignature;

class IDeltaTransformable
{
	
public:
	IDeltaTransformable() = default;
	IDeltaTransformable(const DeltaTransform& Transform);

	IDeltaTransformable(const DeltaTransform& Transform, const std::shared_ptr<IDeltaTransformable> _Parent);

	virtual ~IDeltaTransformable();

	void SetParent(const std::shared_ptr<IDeltaTransformable> _Parent);
	std::shared_ptr<IDeltaTransformable> GetParent() const;

	const DeltaTransform& GetTransform() const;
	const glm::vec3& GetLocation() const;
	const glm::quat& GetRotation() const;
	const glm::vec3& GetScale() const;
	glm::vec3 GetForwardVector() const;
	glm::vec3 GetRightVector() const;
	glm::vec3 GetUpVector() const;

	void SetTransform(const DeltaTransform& InTransform);
	void SetLocation(const glm::vec3& InLocation);
	void SetRotation(const glm::quat& InRotation);
	void SetRotation(const glm::vec3& InRotation);
	void SetDirection(const glm::vec3& Direction);
	void Rotate(const glm::quat& InRotation);
	void SetScale(const glm::vec3& InScale);

	const DeltaTransform& GetTransform_World() const;
	const glm::vec3& GetLocation_World() const;
	const glm::quat& GetRotation_World() const;
	const glm::vec3& GetScale_World() const;
	glm::vec3 GetForwardVector_World() const;
	glm::vec3 GetRightVector_World() const;
	glm::vec3 GetUpVector_World() const;


	void SetTransform_World(const DeltaTransform& InTransform);
	void SetLocation_World(const glm::vec3& InLocation);
	void SetRotation_World(const glm::vec3& InRotation);
	void SetRotation_World(const glm::quat& InRotation);
	void SetDirection_World(const glm::vec3& Direction);
	void SetScale_World(const glm::vec3& InScale);

private:

	void UpdateWorldTransform();

	void OnParentTransformUpdated(const IDeltaTransformable* _parent);

	

public:
	OnTransformUpdatedSignature OnTransformUpdated;

private:
	DeltaTransform LocalTransform;
	DeltaTransform WorldTransform;

	std::weak_ptr<IDeltaTransformable> ParentPtr;
};