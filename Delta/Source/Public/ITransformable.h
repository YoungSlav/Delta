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

		ITransformable(const Transform& Transform, const std::shared_ptr<ITransformable> _Parent);

		virtual ~ITransformable();

		void SetParent(const std::shared_ptr<ITransformable> _Parent);
		std::shared_ptr<ITransformable> GetParent() const;

		const Transform& GetTransform() const;
		const glm::vec3& GetLocation() const;
		const glm::quat& GetRotation() const;
		const glm::vec3& GetScale() const;
		glm::vec3 GetForwardVector() const;
		glm::vec3 GetRightVector() const;
		glm::vec3 GetUpVector() const;

		void SetTransform(const Transform& InTransform);
		void SetLocation(const glm::vec3& InLocation);
		void SetRotation(const glm::quat& InRotation);
		void SetRotation(const glm::vec3& InRotation);
		void SetDirection(const glm::vec3& Direction);
		void Rotate(const glm::quat& InRotation);
		void SetScale(const glm::vec3& InScale);

		const Transform& GetTransform_World() const;
		const glm::vec3& GetLocation_World() const;
		const glm::quat& GetRotation_World() const;
		const glm::vec3& GetScale_World() const;
		glm::vec3 GetForwardVector_World() const;
		glm::vec3 GetRightVector_World() const;
		glm::vec3 GetUpVector_World() const;


		void SetTransform_World(const Transform& InTransform);
		void SetLocation_World(const glm::vec3& InLocation);
		void SetRotation_World(const glm::vec3& InRotation);
		void SetRotation_World(const glm::quat& InRotation);
		void SetDirection_World(const glm::vec3& Direction);
		void SetScale_World(const glm::vec3& InScale);

	private:

		void UpdateWorldTransform();

		void OnParentTransformUpdated(const ITransformable* _parent);

	

	public:
		OnTransformUpdatedSignature OnTransformUpdated;

	private:
		Transform LocalTransform;
		Transform WorldTransform;

		std::weak_ptr<ITransformable> ParentPtr;
	};
}