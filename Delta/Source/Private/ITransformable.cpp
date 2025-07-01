#include "ITransformable.h"

using namespace Delta;

ITransformable::ITransformable(const Transform& Transform) : 
	LocalTransform(Transform),
	WorldTransform(Transform)
{
}

ITransformable::ITransformable(const Transform& Transform, const std::shared_ptr<ITransformable> _Parent) :
	LocalTransform(Transform),
	WorldTransform()
{
	SetParent(_Parent);
}
ITransformable::~ITransformable()
{
	if ( std::shared_ptr<ITransformable> parent = GetParent() )
	{
		parent->OnTransformUpdated.RemoveObject(this);
	}
}

void ITransformable::SetParent(const std::shared_ptr<ITransformable> _Parent)
{
	if ( std::shared_ptr<ITransformable> oldParent = GetParent() )
	{
		oldParent->OnTransformUpdated.RemoveObject(this);
	}
	ParentPtr = _Parent;
	if ( _Parent != nullptr )
	{
		_Parent->OnTransformUpdated.AddRaw(this, &ITransformable::OnParentTransformUpdated);
		OnParentTransformUpdated(_Parent.get());
	}
}
std::shared_ptr<ITransformable> ITransformable::GetParent() const { return ParentPtr.expired() ? nullptr : ParentPtr.lock(); }

const Transform& ITransformable::GetTransform() const { return LocalTransform; }
const glm::vec3& ITransformable::GetLocation() const { return LocalTransform.GetLocation(); }
const glm::quat& ITransformable::GetRotation() const { return LocalTransform.GetRotation(); }
const glm::vec3& ITransformable::GetScale() const { return LocalTransform.GetScale(); }
glm::vec3 ITransformable::GetForwardVector() const { return LocalTransform.GetForwardVector(); }
glm::vec3 ITransformable::GetRightVector() const { return LocalTransform.GetRightVector(); }
glm::vec3 ITransformable::GetUpVector() const { return LocalTransform.GetUpVector(); }

void ITransformable::SetTransform(const Transform& InTransform)
{
	LocalTransform = InTransform;
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::SetLocation(const glm::vec3& InLocation)
{
	LocalTransform.SetLocation(InLocation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::SetRotation(const glm::quat& InRotation)
{
	LocalTransform.SetRotation(InRotation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::SetRotation(const glm::vec3& InRotation)
{
	LocalTransform.SetRotationEuler(InRotation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::SetDirection(const glm::vec3& Direction)
{
	LocalTransform.SetDirection(Direction);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::Rotate(const glm::quat& InRotation)
{
	LocalTransform.Rotate(InRotation);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::SetScale(const glm::vec3& InScale)
{
	LocalTransform.SetScale(InScale);
	UpdateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}

const Transform& ITransformable::GetTransform_World() const
{
	return WorldTransform;
}

const glm::vec3& ITransformable::GetLocation_World() const { return WorldTransform.GetLocation(); }
const glm::quat& ITransformable::GetRotation_World() const { return WorldTransform.GetRotation(); }
const glm::vec3& ITransformable::GetScale_World() const { return WorldTransform.GetScale(); }
glm::vec3 ITransformable::GetForwardVector_World() const { return WorldTransform.GetForwardVector(); }
glm::vec3 ITransformable::GetRightVector_World() const { return WorldTransform.GetRightVector(); }
glm::vec3 ITransformable::GetUpVector_World() const { return WorldTransform.GetUpVector(); }


void ITransformable::SetTransform_World(const Transform& InTransform)
{
	if ( std::shared_ptr<ITransformable> p = GetParent() )
	{
		Transform panretTransform = p->GetTransform_World();
		glm::mat4 parentWorldInverse = glm::inverse(panretTransform.GetTransformMatrix());
		glm::mat4 localTransform = parentWorldInverse * InTransform.GetTransformMatrix();
		SetTransform(Transform(localTransform));
	}
	else
	{
		SetTransform(InTransform);
	}
}
void ITransformable::SetLocation_World(const glm::vec3& InLocation)
{
	Transform worldTransform = GetTransform_World();
	worldTransform.SetLocation(InLocation);
	SetTransform_World(worldTransform);
}
void ITransformable::SetRotation_World(const glm::vec3& InRotation)
{
	Transform worldTransform = GetTransform_World();
	worldTransform.SetRotationEuler(InRotation);
	SetTransform_World(worldTransform);
}
void ITransformable::SetRotation_World(const glm::quat& InRotation)
{
	Transform worldTransform = GetTransform_World();
	worldTransform.SetRotation(InRotation);
	SetTransform_World(worldTransform);
}
void ITransformable::SetDirection_World(const glm::vec3& Direction)
{
	Transform worldTransform = GetTransform_World();
	worldTransform.SetDirection(Direction);
	SetTransform_World(worldTransform);
}
void ITransformable::SetScale_World(const glm::vec3& InScale)
{
	Transform worldTransform = GetTransform_World();
	worldTransform.SetScale(InScale);
	SetTransform_World(worldTransform);
}


void ITransformable::UpdateWorldTransform()
{
	if ( std::shared_ptr<ITransformable> p = GetParent() )
	{
		const Transform& parentTransform = p->GetTransform_World();
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

void ITransformable::OnParentTransformUpdated(const ITransformable* _parent)
{
	UpdateWorldTransform();
}