#pragma once

#include "stdafx.h"
#include "Actor.h"

namespace Delta
{


class Player : public Actor, public ITickable
{
public:
	template <typename... Args>
	Player(Args&&... args) :
		Actor(std::forward<Args>(args)...),
		ITickable()
	{}


	void SetMoveSpeed(float NewSpeed);

protected:
	virtual bool Initialize_Internal() override;

	virtual void Tick(float DeltaTime) override;

	void OnInput_W(bool ButtonState, float DeltaTime);
	void OnInput_S(bool ButtonState, float DeltaTime);
	void OnInput_A(bool ButtonState, float DeltaTime);
	void OnInput_D(bool ButtonState, float DeltaTime);
	void OnInput_E(bool ButtonState, float DeltaTime);
	void OnInput_Q(bool ButtonState, float DeltaTime);
	void OnInput_ESCAPE(bool ButtonState, float DeltaTime);
	void OnInput_MOUSERIGHT(bool ButtonState, float DeltaTime);

	void OnMouseScroll(const glm::vec2& DeltaScroll, float DeltaTime);
	void OnMouseMove(const glm::vec2& CurrentPosition, const glm::vec2& DeltaMove, float DeltaTime);

	void AdjustVelocity(const glm::vec3& DeltaVelocity);

private:
	std::shared_ptr<class Camera> CameraComponent;

	glm::vec3 Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	float MinMoveSpeed = 0.10f;
	float MaxMoveSpeed = 100.00;

	const float InputMoveSensitivity = 2.5f;
	const float InputMouseMoveSensitivity = 0.1f;
	const float InputMouseScrollSensitivity = 0.1f;

	float MoveSpeed = MaxMoveSpeed / 2;

	bool bMouseInput = false;
};
}