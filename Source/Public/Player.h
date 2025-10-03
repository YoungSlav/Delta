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


	void setMoveSpeed(float NewSpeed);

	std::shared_ptr<class Camera> getCamera() const { return cameraComponent; }

protected:
	virtual bool initialize_Internal() override;

	virtual void tick(float DeltaTime) override;

	void onInput_W(bool ButtonState, float DeltaTime);
	void onInput_S(bool ButtonState, float DeltaTime);
	void onInput_A(bool ButtonState, float DeltaTime);
	void onInput_D(bool ButtonState, float DeltaTime);
	void onInput_E(bool ButtonState, float DeltaTime);
	void onInput_Q(bool ButtonState, float DeltaTime);
	void onInput_ESCAPE(bool ButtonState, float DeltaTime);
	void onInput_LEFTCTR(bool ButtonState, float DeltaTime);
	void onInput_MOUSERIGHT(bool ButtonState, float DeltaTime);

	void onMouseScroll(const glm::vec2& DeltaScroll, float DeltaTime);
	void onMouseMove(const glm::vec2& CurrentPosition, const glm::vec2& DeltaMove, float DeltaTime);

	void adjustVelocity(const glm::vec3& DeltaVelocity);

private:
	std::shared_ptr<class Camera> cameraComponent;

	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	float minMoveSpeed = 0.10f;
	float maxMoveSpeed = 100.00;

	const float inputMoveSensitivity = 2.5f;
	const float inputMouseMoveSensitivity = 0.1f;
	const float inputMouseScrollSensitivity = 0.1f;

	float moveSpeed = maxMoveSpeed / 2;

	bool bMouseInput = false;
};
}