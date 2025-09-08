#include "Transform.h"
#include "DeltaMath.h"

using namespace Delta;

Transform::Transform()
{
}

Transform::Transform(const glm::vec3& inLocation, const glm::quat& inRotation, const glm::vec3& inScale) :
	location(inLocation), rotation(inRotation), scale(inScale)
{
	updateTransformMatrix();
}

Transform::Transform(const glm::mat4& inTransform) :
	matrix(inTransform)
{
	decomposeTransformMatrix();
}

glm::vec3 Transform::getForwardVector() const
{
	return glm::rotate(rotation, Math::forwardV);
}

glm::vec3 Transform::getRightVector() const
{
	return glm::rotate(rotation, Math::rightV);
}

glm::vec3 Transform::getUpVector() const
{
	return glm::rotate(rotation, Math::upV);
}


void Transform::setTransformMatrix(const glm::mat4& inTransform)
{
	matrix = inTransform;
	decomposeTransformMatrix();
}

void Transform::setLocation(const glm::vec3& inLocation)
{
	location = inLocation;
	bMatrixDirty = true;
}

void Transform::setRotation(const glm::quat& inRotation)
{
	rotation = inRotation;
	bMatrixDirty = true;
}

void Transform::setRotationEuler(const glm::vec3& inEulerRotation)
{
	rotationEuler = inEulerRotation;
	rotation = Math::eulerToQuat(inEulerRotation);
	bMatrixDirty = true;
}

const glm::vec3& Transform::getLocation() const
{
	return location;
}
const glm::quat& Transform::getRotation() const
{
	return rotation;
}
const glm::vec3& Transform::getRotationEuler() const
{
	rotationEuler = Math::quatToEuler(rotation);
	return rotationEuler;
}
const glm::vec3& Transform::getScale() const
{
	return scale;
}
const glm::mat4& Transform::getTransformMatrix() const
{
	updateTransformMatrix();
	return matrix;
}


void Transform::rotate(const glm::quat& inRotation)
{
	rotation = inRotation * rotation;
	bMatrixDirty = true;
}
	
void Transform::setDirection(const glm::vec3& inDirection)
{
	glm::vec3 dir(0.0f);
	if ( glm::length(inDirection) <= FLT_EPS )
		dir = Math::forwardV;
	else
		dir = glm::normalize(inDirection);

	rotation = Math::directionToQuaternion(dir);
	bMatrixDirty = true;
}

void Transform::setScale(const glm::vec3& inScale)
{
	scale = inScale;
	bMatrixDirty = true;
}

void Transform::updateTransformMatrix() const
{
	if ( !bMatrixDirty ) return;
		
	matrix = glm::recompose(scale, rotation, location, glm::vec3(0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	bMatrixDirty = false;
}

void Transform::decomposeTransformMatrix()
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, rotation, location, skew, perspective);

	bMatrixDirty = false;
}