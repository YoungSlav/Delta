#include "Transform.h"
#include "DeltaMath.h"

using namespace Delta;

Transform::Transform()
{
}

Transform::Transform(const glm::vec3& _location, const glm::quat& _rotation, const glm::vec3& _scale) :
	Location(_location), Rotation(_rotation), Scale(_scale)
{
	UpdateTransformMatrix();
}

Transform::Transform(const glm::mat4& _transform) :
	TransformMatrix(_transform)
{
	DecomposeTransformMatrix();
}

glm::vec3 Transform::GetForwardVector() const
{
	return glm::rotate(Rotation, Math::forwardV);
}

glm::vec3 Transform::GetRightVector() const
{
	return glm::rotate(Rotation, Math::rightV);
}

glm::vec3 Transform::GetUpVector() const
{
	return glm::rotate(Rotation, Math::upV);
}


void Transform::SetTransformMatrix(const glm::mat4& _transform)
{
	TransformMatrix = _transform;
	DecomposeTransformMatrix();
}

void Transform::SetLocation(const glm::vec3& _location)
{
	Location = _location;
	bMatrixDirty = true;
}

void Transform::SetRotation(const glm::quat& _rotation)
{
	Rotation = _rotation;
	bMatrixDirty = true;
}

void Transform::SetRotationEuler(const glm::vec3& _eulerRotation)
{
	RotationEuler = _eulerRotation;
	Rotation = Math::EulerToQuat(_eulerRotation);
	bMatrixDirty = true;
}

const glm::vec3& Transform::GetLocation() const
{
	return Location;
}
const glm::quat& Transform::GetRotation() const
{
	return Rotation;
}
const glm::vec3& Transform::GetRotationEuler() const
{
	RotationEuler = Math::QuatToEuler(Rotation);
	return RotationEuler;
}
const glm::vec3& Transform::GetScale() const
{
	return Scale;
}
const glm::mat4& Transform::GetTransformMatrix() const
{
	UpdateTransformMatrix();
	return TransformMatrix;
}


void Transform::Rotate(const glm::quat& _rotation)
{
	Rotation = _rotation * Rotation;
	bMatrixDirty = true;
}
	
void Transform::SetDirection(const glm::vec3& _direction)
{
	glm::vec3 dir(0.0f);
	if ( glm::length(_direction) <= FLT_EPS )
		dir = Math::forwardV;
	else
		dir = glm::normalize(_direction);

	Rotation = Math::DirectionToQuaternion(dir);
	bMatrixDirty = true;
}

void Transform::SetScale(const glm::vec3& _scale)
{
	Scale = _scale;
	bMatrixDirty = true;
}

void Transform::UpdateTransformMatrix() const
{
	if ( !bMatrixDirty ) return;
		
	TransformMatrix = glm::recompose(Scale, Rotation, Location, glm::vec3(0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	bMatrixDirty = false;
}

void Transform::DecomposeTransformMatrix()
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(TransformMatrix, Scale, Rotation, Location, skew, perspective);

	bMatrixDirty = false;
}