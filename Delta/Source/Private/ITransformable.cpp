#include "ITransformable.h"

using namespace Delta;

ITransformable::ITransformable(const Transform& Transform) : 
	localTransform(Transform),
	worldTransform(Transform)
{
}

ITransformable::ITransformable(const Transform& Transform, const std::shared_ptr<ITransformable> inParent) :
	localTransform(Transform),
	worldTransform()
{
	setParent(inParent);
}
ITransformable::~ITransformable()
{
	if ( std::shared_ptr<ITransformable> parent = getParent() )
	{
		parent->OnTransformUpdated.RemoveObject(this);
	}
}

void ITransformable::setParent(const std::shared_ptr<ITransformable> inParent)
{
	if ( std::shared_ptr<ITransformable> oldParent = getParent() )
	{
		oldParent->OnTransformUpdated.RemoveObject(this);
	}
	parent = inParent;
	if ( inParent != nullptr )
	{
		inParent->OnTransformUpdated.AddRaw(this, &ITransformable::onParentTransformUpdated);
		onParentTransformUpdated(inParent.get());
	}
}
std::shared_ptr<ITransformable> ITransformable::getParent() const { return parent.expired() ? nullptr : parent.lock(); }

const Transform& ITransformable::getTransform() const { return localTransform; }
const glm::vec3& ITransformable::getLocation() const { return localTransform.getLocation(); }
const glm::quat& ITransformable::getRotation() const { return localTransform.getRotation(); }
const glm::vec3& ITransformable::getScale() const { return localTransform.getScale(); }
glm::vec3 ITransformable::getForwardVector() const { return localTransform.getForwardVector(); }
glm::vec3 ITransformable::getRightVector() const { return localTransform.getRightVector(); }
glm::vec3 ITransformable::getUpVector() const { return localTransform.getUpVector(); }

void ITransformable::setTransform(const Transform& InTransform)
{
	localTransform = InTransform;
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::setLocation(const glm::vec3& InLocation)
{
	localTransform.setLocation(InLocation);
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::setRotation(const glm::quat& InRotation)
{
	localTransform.setRotation(InRotation);
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::setRotation(const glm::vec3& InRotation)
{
	localTransform.setRotationEuler(InRotation);
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::setDirection(const glm::vec3& Direction)
{
	localTransform.setDirection(Direction);
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::rotate(const glm::quat& InRotation)
{
	localTransform.rotate(InRotation);
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}
void ITransformable::setScale(const glm::vec3& InScale)
{
	localTransform.setScale(InScale);
	updateWorldTransform();
	OnTransformUpdated.Broadcast(this);
}

const Transform& ITransformable::getTransform_World() const
{
	return worldTransform;
}

const glm::vec3& ITransformable::getLocation_World() const { return worldTransform.getLocation(); }
const glm::quat& ITransformable::getRotation_World() const { return worldTransform.getRotation(); }
const glm::vec3& ITransformable::getScale_World() const { return worldTransform.getScale(); }
glm::vec3 ITransformable::getForwardVector_World() const { return worldTransform.getForwardVector(); }
glm::vec3 ITransformable::getRightVector_World() const { return worldTransform.getRightVector(); }
glm::vec3 ITransformable::getUpVector_World() const { return worldTransform.getUpVector(); }


void ITransformable::setTransform_World(const Transform& InTransform)
{
	if ( std::shared_ptr<ITransformable> p = getParent() )
	{
		Transform panretTransform = p->getTransform_World();
		glm::mat4 parentWorldInverse = glm::inverse(panretTransform.getTransformMatrix());
		glm::mat4 localTransform = parentWorldInverse * InTransform.getTransformMatrix();
		setTransform(Transform(localTransform));
	}
	else
	{
		setTransform(InTransform);
	}
}
void ITransformable::setLocation_World(const glm::vec3& InLocation)
{
	Transform worldTransform = getTransform_World();
	worldTransform.setLocation(InLocation);
	setTransform_World(worldTransform);
}
void ITransformable::setRotation_World(const glm::vec3& InRotation)
{
	Transform worldTransform = getTransform_World();
	worldTransform.setRotationEuler(InRotation);
	setTransform_World(worldTransform);
}
void ITransformable::setRotation_World(const glm::quat& InRotation)
{
	Transform worldTransform = getTransform_World();
	worldTransform.setRotation(InRotation);
	setTransform_World(worldTransform);
}
void ITransformable::setDirection_World(const glm::vec3& Direction)
{
	Transform worldTransform = getTransform_World();
	worldTransform.setDirection(Direction);
	setTransform_World(worldTransform);
}
void ITransformable::setScale_World(const glm::vec3& InScale)
{
	Transform worldTransform = getTransform_World();
	worldTransform.setScale(InScale);
	setTransform_World(worldTransform);
}


void ITransformable::updateWorldTransform()
{
	if ( std::shared_ptr<ITransformable> p = getParent() )
	{
		const Transform& parentTransform = p->getTransform_World();
		glm::mat4 parentMat = parentTransform.getTransformMatrix();
		glm::mat4 myMat = localTransform.getTransformMatrix();
		glm::mat4 myWorld = parentMat * myMat;
		worldTransform.setTransformMatrix(myWorld);
	}
	else
	{
		worldTransform = localTransform;
	}
}

void ITransformable::onParentTransformUpdated(const ITransformable* inparent)
{
	updateWorldTransform();
}