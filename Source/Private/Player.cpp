#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Camera.h"
#include "DeltaMath.h"

using namespace Delta;

bool Player::initialize_Internal()
{
	if ( std::shared_ptr<Input> InputPtr = engine->getInput() )
	{
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_W;
		subscription.subscribedType = EKeySubscriptionType::HOLD;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_W);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_S;
		subscription.subscribedType = EKeySubscriptionType::HOLD;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_S);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_A;
		subscription.subscribedType = EKeySubscriptionType::HOLD;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_A);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_D;
		subscription.subscribedType = EKeySubscriptionType::HOLD;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_D);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_E;
		subscription.subscribedType = EKeySubscriptionType::HOLD;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_E);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_Q;
		subscription.subscribedType = EKeySubscriptionType::HOLD;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_Q);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_MOUSE_BUTTON_RIGHT;
		subscription.subscribedType = EKeySubscriptionType::HOLD | EKeySubscriptionType::RELEASED;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_MOUSERIGHT);
		InputPtr->subscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.key = GLFW_KEY_ESCAPE;
		subscription.subscribedType = EKeySubscriptionType::RELEASED;
		subscription.callback.BindSP(Self<Player>(), &Player::onInput_ESCAPE);
		InputPtr->subscribeKey(subscription);
		}
		{
			InputPtr->OnMouseMoveDelegate.AddSP(Self<Player>(), &Player::onMouseMove);
		}
		{
			InputPtr->OnMouseScrollDelegate.AddSP(Self<Player>(), &Player::onMouseScroll);
		}
	}

	cameraComponent = spawn<Camera>("PlayerCamera");

	return true;
}

void Player::tick(float DeltaTime)
{
	setLocation(getLocation() + velocity * moveSpeed * DeltaTime);
	velocity = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Player::onInput_W(bool ButtonState, float DeltaTime)
{
	adjustVelocity(getForwardVector_World());
}
void Player::onInput_S(bool ButtonState, float DeltaTime)
{
	adjustVelocity(-getForwardVector_World());
}
void Player::onInput_A(bool ButtonState, float DeltaTime)
{
	glm::vec3 right = glm::normalize(glm::cross(Math::upV, getForwardVector_World()));
	adjustVelocity(right);
}
void Player::onInput_D(bool ButtonState, float DeltaTime)
{
	glm::vec3 right = glm::normalize(glm::cross(Math::upV, getForwardVector_World()));
	adjustVelocity(-right);
}
void Player::onInput_E(bool ButtonState, float DeltaTime)
{
	glm::vec3 up = Math::upV;
	adjustVelocity(up);
}
void Player::onInput_Q(bool ButtonState, float DeltaTime)
{
	glm::vec3 up = Math::upV;
	adjustVelocity(-up);
}
void Player::adjustVelocity(const glm::vec3& DeltaVelocity)
{
	velocity += (DeltaVelocity * inputMoveSensitivity);
}
void Player::onInput_ESCAPE(bool ButtonState, float DeltaTime)
{
	engine->shutDown();
}
void Player::onInput_MOUSERIGHT(bool ButtonState, float DeltaTime)
{
	bMouseInput = ButtonState;
}

void Player::onMouseScroll(const glm::vec2& DeltaScroll, float DeltaTime)
{
	moveSpeed = glm::clamp(moveSpeed + inputMouseScrollSensitivity * DeltaScroll.y, minMoveSpeed, maxMoveSpeed);
}

void Player::setMoveSpeed(float NewSpeed)
{
	moveSpeed = glm::clamp(NewSpeed, minMoveSpeed, maxMoveSpeed);
}
void Player::onMouseMove(const glm::vec2& CurrentPosition, const glm::vec2& DeltaMove, float DeltaTime)
{
	if ( !bMouseInput || glm::all(glm::epsilonEqual(DeltaMove, glm::vec2(0.0f), FLT_EPS)) ) return;

	glm::quat currentRotation = getRotation_World();
		
	float yawDelta = glm::radians(DeltaMove.x * -inputMouseMoveSensitivity);
	glm::quat yawRotation = glm::angleAxis(yawDelta, Math::upV);
	
	float pitchDelta = glm::radians(DeltaMove.y * -inputMouseMoveSensitivity);
	glm::quat pitchRotation = glm::normalize(glm::angleAxis(pitchDelta, getRightVector_World()));

	glm::quat newRotation = glm::normalize(pitchRotation * yawRotation * currentRotation);

	const glm::vec3 newCameraDirection = Math::quaternionToDirection(newRotation);
	const float MaxPitch = 89.0f;
	const float maxDot = glm::abs(glm::cos(glm::radians(MaxPitch)));
	const float newDot = 1.0f - glm::abs(glm::dot(newCameraDirection, Math::upV));

	if ( newDot <= maxDot )
	{
		const glm::vec3 currentCameraDirection = Math::quaternionToDirection(currentRotation);
		const float currentDot = 1.0f - glm::abs(glm::dot(currentCameraDirection, Math::upV));

		if ( newDot <= currentDot )
		{
			newRotation = glm::normalize(yawRotation * currentRotation);
		}
	}
	setRotation(newRotation);
}