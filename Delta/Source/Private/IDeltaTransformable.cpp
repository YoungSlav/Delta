#include "IDeltaTransformable.h"

IDeltaTransformable::IDeltaTransformable(const DeltaTransform& Transform) : 
	LocalTransform(Transform),
	WorldTransform(Transform)
{
}

IDeltaTransformable::IDeltaTransformable(const DeltaTransform& Transform, const std::shared_ptr<IDeltaTransformable> _Parent) :
	LocalTransform(Transform),
	WorldTransform()
{
	SetParent(_Parent);
}
IDeltaTransformable::~IDeltaTransformable()
{
	if ( std::shared_ptr<IDeltaTransformable> parent = GetParent() )
	{
		parent->OnTransformUpdated.RemoveObject(this);
	}
}

void IDeltaTransformable::SetParent(const std::shared_ptr<IDeltaTransformable> _Parent)
{
	if ( std::shared_ptr<IDeltaTransformable> oldParent = GetParent() )
	{
		oldParent->OnTransformUpdated.RemoveObject(this);
	}
	ParentPtr = _Parent;
	if ( _Parent != nullptr )
	{
		_Parent->OnTransformUpdated.AddRaw(this, &IDeltaTransformable::OnParentTransformUpdated);
		OnParentTransformUpdated(_Parent.get());
	}
}
std::shared_ptr<IDeltaTransformable> IDeltaTransformable::GetParent() const { return ParentPtr.expired() ? nullptr : ParentPtr.lock(); }

const DeltaTransform& IDeltaTransformable::GetTransform() const { return LocalTransform; }
const glm::vec3& IDeltaTransformable::GetLocation() const { return LocalTransform.GetLocation(); }
const glm::quat& IDeltaTransformable::GetRotation() const { return LocalTransform.GetRotation(); }
const glm::vec3& IDeltaTransformable::GetScale() const { return LocalTransform.GetScale(); }
glm::vec3 IDeltaTransformable::GetForwardVector() const { return LocalTransform.GetForwardVector(); }
glm::vec3 IDeltaTransformable::GetRightVector() const { return LocalTransform.GetRightVector(); }
glm::vec3 IDeltaTransformable::GetUpVector() const { return LocalTransform.GetUpVector(); }

void IDeltaTransformable::SetTransform(const DeltaTransform& InTransform)
{
	LocalTransform = InTransform;
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void IDeltaTransformable::SetLocation(const glm::vec3& InLocation)
{
	LocalTransform.SetLocation(InLocation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void IDeltaTransformable::SetRotation(const glm::quat& InRotation)
{
	LocalTransform.SetRotation(InRotation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void IDeltaTransformable::SetRotation(const glm::vec3& InRotation)
{
	LocalTransform.SetRotationEuler(InRotation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void IDeltaTransformable::SetDirection(const glm::vec3& Direction)
{
	LocalTransform.SetDirection(Direction);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void IDeltaTransformable::Rotate(const glm::quat& InRotation)
{
	LocalTransform.Rotate(InRotation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void IDeltaTransformable::SetScale(const glm::vec3& InScale)
{
	LocalTransform.SetScale(InScale);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}

const DeltaTransform& IDeltaTransformable::GetTransform_World() const
{
	return WorldTransform;
}

const glm::vec3& IDeltaTransformable::GetLocation_World() const { return WorldTransform.GetLocation(); }
const glm::quat& IDeltaTransformable::GetRotation_World() const { return WorldTransform.GetRotation(); }
const glm::vec3& IDeltaTransformable::GetScale_World() const { return WorldTransform.GetScale(); }
glm::vec3 IDeltaTransformable::GetForwardVector_World() const { return WorldTransform.GetForwardVector(); }
glm::vec3 IDeltaTransformable::GetRightVector_World() const { return WorldTransform.GetRightVector(); }
glm::vec3 IDeltaTransformable::GetUpVector_World() const { return WorldTransform.GetUpVector(); }


void IDeltaTransformable::SetTransform_World(const DeltaTransform& InTransform)
{
	if ( std::shared_ptr<IDeltaTransformable> p = GetParent() )
	{
		DeltaTransform panretTransform = p->GetTransform_World();
		glm::mat4 parentWorldInverse = glm::inverse(panretTransform.GetTransformMatrix());
		glm::mat4 localTransform = parentWorldInverse * InTransform.GetTransformMatrix();
		SetTransform(DeltaTransform(localTransform));
	}
	else
	{
		SetTransform(InTransform);
	}
}
void IDeltaTransformable::SetLocation_World(const glm::vec3& InLocation)
{
	DeltaTransform worldTransform = GetTransform_World();
	worldTransform.SetLocation(InLocation);
	SetTransform_World(worldTransform);
}
void IDeltaTransformable::SetRotation_World(const glm::vec3& InRotation)
{
	DeltaTransform worldTransform = GetTransform_World();
	worldTransform.SetRotationEuler(InRotation);
	SetTransform_World(worldTransform);
}
void IDeltaTransformable::SetRotation_World(const glm::quat& InRotation)
{
	DeltaTransform worldTransform = GetTransform_World();
	worldTransform.SetRotation(InRotation);
	SetTransform_World(worldTransform);
}
void IDeltaTransformable::SetDirection_World(const glm::vec3& Direction)
{
	DeltaTransform worldTransform = GetTransform_World();
	worldTransform.SetDirection(Direction);
	SetTransform_World(worldTransform);
}
void IDeltaTransformable::SetScale_World(const glm::vec3& InScale)
{
	DeltaTransform worldTransform = GetTransform_World();
	worldTransform.SetScale(InScale);
	SetTransform_World(worldTransform);
}


void IDeltaTransformable::UpdateWorldTransform()
{
	if ( std::shared_ptr<IDeltaTransformable> p = GetParent() )
	{
		const DeltaTransform& parentTransform = p->GetTransform_World();
		glm::mat4 parentMat = parentTransform.GetTransformMatrix();
		glm::mat4 myMat = LocalTransform.GetTransformMatrix();
		glm::mat4 myWorld = parentMat * myMat;
		WorldTransform.SetTransformMatrix(myWorld);
	}
	else
	{
		WorldTransform = LocalTransform;
	}
}

void IDeltaTransformable::OnParentTransformUpdated(const IDeltaTransformable* _parent)
{
	UpdateWorldTransform();
}