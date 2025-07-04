#include "Transform.h"
#include "DeltaMath.h"

using namespace Delta;

Transform::Transform()
{
}

Transform::Transform(const glm::vec3& _location, const glm::quat& _rotation, const glm::vec3& _scale) :
	location(_location), rotation(_rotation), scale(_scale)
{
	updateTransformMatrix();
}

Transform::Transform(const glm::mat4& _transform) :
	matrix(_transform)
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


void Transform::setTransformMatrix(const glm::mat4& _transform)
{
	matrix = _transform;
	decomposeTransformMatrix();
}

void Transform::setLocation(const glm::vec3& _location)
{
	location = _location;
	bMatrixDirty = true;
}

void Transform::setRotation(const glm::quat& _rotation)
{
	rotation = _rotation;
	bMatrixDirty = true;
}

void Transform::setRotationEuler(const glm::vec3& _eulerRotation)
{
	rotationEuler = _eulerRotation;
	rotation = Math::eulerToQuat(_eulerRotation);
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


void Transform::rotate(const glm::quat& _rotation)
{
	rotation = _rotation * rotation;
	bMatrixDirty = true;
}
	
void Transform::setDirection(const glm::vec3& _direction)
{
	glm::vec3 dir(0.0f);
	if ( glm::length(_direction) <= FLT_EPS )
		dir = Math::forwardV;
	else
		dir = glm::normalize(_direction);

	rotation = Math::directionToQuaternion(dir);
	bMatrixDirty = true;
}

void Transform::setScale(const glm::vec3& _scale)
{
	scale = _scale;
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