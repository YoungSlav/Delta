#pragma once

#include "stdafx.h"

namespace Delta
{

class Transform final
{
public:
	Transform();

	Transform(const glm::vec3& inLocation, const glm::quat& inRotation, const glm::vec3& inScale);
	Transform(const glm::mat4& intransform);

	const glm::vec3& getLocation() const;
	const glm::quat& getRotation() const;
	const glm::vec3& getRotationEuler() const;
	const glm::vec3& getScale() const;
	const glm::mat4& getTransformMatrix() const;

	glm::vec3 getForwardVector() const;
	glm::vec3 getRightVector() const;
	glm::vec3 getUpVector() const;


	void setTransformMatrix(const glm::mat4& intransform);
	void setLocation(const glm::vec3& inlocation);
	void setRotation(const glm::quat& inrotation);
	void setRotationEuler(const glm::vec3& ineulerRotation);

	void rotate(const glm::quat& inrotation);
	void setDirection(const glm::vec3& indirection);
	void setScale(const glm::vec3& inscale);

private:
	void updateTransformMatrix() const;
	void decomposeTransformMatrix();

private:
	mutable glm::mat4 matrix = glm::mat4(1.0f);
	mutable bool bMatrixDirty = true;

	glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat rotation = glm::quat();
	mutable glm::vec3 rotationEuler = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
};

}