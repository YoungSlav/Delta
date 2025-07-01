#include "DeltaTransform.h"
#include "DeltaMath.h"

DeltaTransform::DeltaTransform()
{
}

DeltaTransform::DeltaTransform(const glm::vec3& _location, const glm::quat& _rotation, const glm::vec3& _scale) :
	Location(_location), Rotation(_rotation), Scale(_scale)
{
	UpdateTransformMatrix();
}

DeltaTransform::DeltaTransform(const glm::mat4& _transform) :
	TransformMatrix(_transform)
{
	DecomposeTransformMatrix();
}

glm::vec3 DeltaTransform::GetForwardVector() const
{
	return glm::rotate(Rotation, DeltaMath::forwardV);
}

glm::vec3 DeltaTransform::GetRightVector() const
{
	return glm::rotate(Rotation, DeltaMath::rightV);
}

glm::vec3 DeltaTransform::GetUpVector() const
{
	return glm::rotate(Rotation, DeltaMath::upV);
}


void DeltaTransform::SetTransformMatrix(const glm::mat4& _transform)
{
	TransformMatrix = _transform;
	DecomposeTransformMatrix();
}

void DeltaTransform::SetLocation(const glm::vec3& _location)
{
	Location = _location;
	bMatrixDirty = true;
}

void DeltaTransform::SetRotation(const glm::quat& _rotation)
{
	Rotation = _rotation;
	bMatrixDirty = true;
}

void DeltaTransform::SetRotationEuler(const glm::vec3& _eulerRotation)
{
	RotationEuler = _eulerRotation;
	Rotation = DeltaMath::EulerToQuat(_eulerRotation);
	bMatrixDirty = true;
}

const glm::vec3& DeltaTransform::GetLocation() const
{
	return Location;
}
const glm::quat& DeltaTransform::GetRotation() const
{
	return Rotation;
}
const glm::vec3& DeltaTransform::GetRotationEuler() const
{
	RotationEuler = DeltaMath::QuatToEuler(Rotation);
	return RotationEuler;
}
const glm::vec3& DeltaTransform::GetScale() const
{
	return Scale;
}
const glm::mat4& DeltaTransform::GetTransformMatrix() const
{
	UpdateTransformMatrix();
	return TransformMatrix;
}


void DeltaTransform::Rotate(const glm::quat& _rotation)
{
	Rotation = _rotation * Rotation;
	bMatrixDirty = true;
}
	
void DeltaTransform::SetDirection(const glm::vec3& _direction)
{
	glm::vec3 dir(0.0f);
	if ( glm::length(_direction) <= FLT_EPS )
		dir = DeltaMath::forwardV;
	else
		dir = glm::normalize(_direction);

	Rotation = DeltaMath::DirectionToQuaternion(dir);
	bMatrixDirty = true;
}

void DeltaTransform::SetScale(const glm::vec3& _scale)
{
	Scale = _scale;
	bMatrixDirty = true;
}

void DeltaTransform::UpdateTransformMatrix() const
{
	if ( !bMatrixDirty ) return;
		
	TransformMatrix = glm::recompose(Scale, Rotation, Location, glm::vec3(0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	bMatrixDirty = false;
}

void DeltaTransform::DecomposeTransformMatrix()
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(TransformMatrix, Scale, Rotation, Location, skew, perspective);

	bMatrixDirty = false;
}