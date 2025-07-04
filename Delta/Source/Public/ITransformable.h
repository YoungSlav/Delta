#pragma once
#include "stdafx.h"
#include "Transform.h"

namespace Delta
{
typedef MulticastDelegate<const class ITransformable*> OnTransformUpdatedSignature;

class ITransformable
{
	
public:
	ITransformable() = default;
	ITransformable(const Transform& Transform);

	ITransformable(const Transform& Transform, const std::shared_ptr<ITransformable> inParent);

	virtual ~ITransformable();

	void setParent(const std::shared_ptr<ITransformable> inParent);
	std::shared_ptr<ITransformable> getParent() const;

	const Transform& getTransform() const;
	const glm::vec3& getLocation() const;
	const glm::quat& getRotation() const;
	const glm::vec3& getScale() const;
	glm::vec3 getForwardVector() const;
	glm::vec3 getRightVector() const;
	glm::vec3 getUpVector() const;

	void setTransform(const Transform& InTransform);
	void setLocation(const glm::vec3& InLocation);
	void setRotation(const glm::quat& InRotation);
	void setRotation(const glm::vec3& InRotation);
	void setDirection(const glm::vec3& Direction);
	void rotate(const glm::quat& InRotation);
	void setScale(const glm::vec3& InScale);

	const Transform& getTransform_World() const;
	const glm::vec3& getLocation_World() const;
	const glm::quat& getRotation_World() const;
	const glm::vec3& getScale_World() const;
	glm::vec3 getForwardVector_World() const;
	glm::vec3 getRightVector_World() const;
	glm::vec3 getUpVector_World() const;


	void setTransform_World(const Transform& InTransform);
	void setLocation_World(const glm::vec3& InLocation);
	void setRotation_World(const glm::vec3& InRotation);
	void setRotation_World(const glm::quat& InRotation);
	void setDirection_World(const glm::vec3& Direction);
	void setScale_World(const glm::vec3& InScale);

private:

	void updateWorldTransform();

	void onParentTransformUpdated(const ITransformable* inparent);

	

public:
	OnTransformUpdatedSignature OnTransformUpdated;

private:
	Transform localTransform;
	Transform worldTransform;

	std::weak_ptr<ITransformable> parent;
};
}