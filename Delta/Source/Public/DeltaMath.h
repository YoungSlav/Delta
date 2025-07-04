#pragma once

#include "stdafx.h"

#define FLT_EPS FLT_EPSILON

namespace Delta
{
class Math
{
public:
	static glm::quat directionToQuaternion(const glm::vec3& Direction);
	static glm::vec3 quaternionToDirection(const glm::quat& Quaternion);


	static glm::vec3 normalizeEuler(const glm::vec3& angles);
	static glm::vec3 quatToEuler(const glm::quat& QuatRotation);
	static glm::quat eulerToQuat(const glm::vec3& EulerRotation);

	template<typename T>
	static T randRange(T Min, T Max)
	{
		double r = (double)(rand()) / (double)(RAND_MAX);

		return Min + (T)((Max - Min)*r);
	}
	static glm::vec3 randVector(float MaxDimension);

	static bool isNearlyZero(float Number);
	static bool isNearlyZero(double Number);
	static bool isNearlyZero(const glm::vec2& vector);
	static bool isNearlyZero(const glm::vec3& vector);

	static bool isNearlyEqual(const float& A, const float& B);
	static bool isNearlyEqual(const double& A, const double& B);
	static bool isNearlyEqual(const glm::vec3& A, const glm::vec3& B);
	static bool isNearlyEqual(const glm::quat& A, const glm::quat& B);
	static bool isNearlyEqual(const glm::mat4& A, const glm::mat4& B);
	static bool isNearlyEqual(const class Transform& A, const Transform& B);



	static float maxComponent(const glm::vec3& Vector);

	static void getFrustumCornersWorldSpace(const glm::mat4& frustrum, std::vector<glm::vec4>& OutFrustumCorners);

	static const glm::vec3 upV;
	static const glm::vec3 forwardV;
	static const glm::vec3 rightV;
};

}