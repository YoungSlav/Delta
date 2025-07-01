#include "DeltaMath.h"
#include "Transform.h"

using namespace Delta;

const glm::vec3 Math::forwardV = glm::vec3(0, 0, -1.0);
const glm::vec3 Math::upV = glm::vec3(0.0, 1.0, 0.0);
const glm::vec3 Math::rightV = glm::vec3(1.0, 0.0, 0.0);

glm::vec3 Math::NormalizeEuler(const glm::vec3& euler)
{
	// Convert Euler angles to rotation matrix
	glm::mat4 rotationMatrix = glm::eulerAngleYXZ(glm::radians(euler.y), glm::radians(euler.x), glm::radians(euler.z));

	// Extract pitch, yaw, and roll from the rotation matrix
	float newPitch, newYaw, newRoll;
	glm::extractEulerAngleYXZ(rotationMatrix, newYaw, newPitch, newRoll);

	// Return adjusted pitch and yaw with roll set to 0
	return glm::degrees(glm::vec3(newPitch, newYaw, newRoll));
}


glm::quat Math::DirectionToQuaternion(const glm::vec3& Direction)
{
	glm::vec3 dir = Direction;
	if ( glm::length(dir) <= FLT_EPS )
		dir = Math::forwardV;

	if(glm::abs(glm::dot(dir, Math::upV)) > 1.0f - FLT_EPS)
	{
		return glm::quatLookAt(glm::normalize(dir), Math::forwardV);
	}
	else
	{
		return glm::quatLookAt(glm::normalize(dir), Math::upV);
	}
}

glm::vec3 Math::QuaternionToDirection(const glm::quat& Quaternion)
{
	return glm::rotate(Quaternion, Math::forwardV);
}

glm::vec3 Math::QuatToEuler(const glm::quat& QuatRotation)
{
	glm::vec3 euler = glm::degrees(glm::eulerAngles(QuatRotation));


	return euler;
}
glm::quat Math::EulerToQuat(const glm::vec3& EulerRotation)
{
	return glm::quat(glm::radians(EulerRotation));
}

glm::vec3 Math::RandVector(float MaxDimension)
{
	int32 iMaxDimension = (int32)MaxDimension;
	glm::vec3 Result(
		Math::Rand(-MaxDimension, MaxDimension),
		Math::Rand(-MaxDimension, MaxDimension),
		Math::Rand(-MaxDimension, MaxDimension)
	);
	return Result;
}

bool Math::IsNearlyZero(float Number)
{
	return std::abs(Number) <= FLT_EPS;
}
bool Math::IsNearlyZero(double Number)
{
	return std::abs(Number) <= FLT_EPS;
}
bool Math::IsNearlyZero(const glm::vec2& vector)
{
	return IsNearlyZero(vector.x) && IsNearlyZero(vector.y);
}
bool Math::IsNearlyZero(const glm::vec3& vector)
{
	return IsNearlyZero(vector.x) && IsNearlyZero(vector.y) && IsNearlyZero(vector.z);
}

bool Math::IsNearlyEqual(const float& A, const float& B)
{
	return glm::abs(A-B) <= FLT_EPS;
}
bool Math::IsNearlyEqual(const double& A, const double& B)
{
	return glm::abs(A-B) <= FLT_EPS;
}
bool Math::IsNearlyEqual(const glm::vec3& A, const glm::vec3& B)
{
	return glm::all(glm::epsilonEqual(A,B, glm::vec3(FLT_EPS)));
}
bool Math::IsNearlyEqual(const glm::quat& A, const glm::quat& B)
{
	float dotProduct = glm::dot(A, B);
	return glm::abs(dotProduct) > 1.0f - FLT_EPS;
}
bool Math::IsNearlyEqual(const glm::mat4& A, const glm::mat4& B)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (!glm::epsilonEqual(A[i][j], B[i][j], FLT_EPS))
			{
				return false;
			}
		}
	}
	return true; 
}

bool Math::IsNearlyEqual(const class Transform& A, const Transform& B)
{
	return IsNearlyEqual(A.GetTransformMatrix(), B.GetTransformMatrix());
}

float Math::MaxComponent(const glm::vec3& Vector)
{
	if ( std::abs(Vector.x) > std::abs(Vector.y) && std::abs(Vector.x) > std::abs(Vector.z) )
		return Vector.x;
	if ( std::abs(Vector.y) > std::abs(Vector.x) && std::abs(Vector.y) > std::abs(Vector.z) )
		return Vector.y;
	if ( std::abs(Vector.z) > std::abs(Vector.y) && std::abs(Vector.z) > std::abs(Vector.x) )
		return Vector.z;
	return Vector.z;
}

void Math::GetFrustumCornersWorldSpace(const glm::mat4& frustrum, std::vector<glm::vec4>& OutFrustumCorners)
{
	OutFrustumCorners.clear();
	OutFrustumCorners.reserve(8);

	const auto inv = glm::inverse(frustrum);
	for (int32 x = 0; x < 2; ++x)
	{
		for (int32 y = 0; y < 2; ++y)
		{
			for (int32 z = 0; z < 2; ++z)
			{
				const glm::vec4 pt = 
					inv * glm::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				OutFrustumCorners.push_back(pt / pt.w);
			}
		}
	}
}