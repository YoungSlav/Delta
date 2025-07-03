#include "Player.h"
#include "Engine.h"
#include "Input.h"
#include "Camera.h"
#include "DeltaMath.h"

using namespace Delta;

bool Player::Initialize_Internal()
{
	if ( std::shared_ptr<Input> InputPtr = EnginePtr->GetInput() )
	{
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_W;
		subscription.SubscribedType = EKeySubscriptionType::Hold;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_W);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_S;
		subscription.SubscribedType = EKeySubscriptionType::Hold;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_S);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_A;
		subscription.SubscribedType = EKeySubscriptionType::Hold;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_A);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_D;
		subscription.SubscribedType = EKeySubscriptionType::Hold;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_D);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_E;
		subscription.SubscribedType = EKeySubscriptionType::Hold;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_E);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_Q;
		subscription.SubscribedType = EKeySubscriptionType::Hold;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_Q);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_MOUSE_BUTTON_RIGHT;
		subscription.SubscribedType = EKeySubscriptionType::Hold | EKeySubscriptionType::Released;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_MOUSERIGHT);
		InputPtr->SubscribeKey(subscription);
		}
		{
		KeySubscription subscription;
		subscription.Key = GLFW_KEY_ESCAPE;
		subscription.SubscribedType = EKeySubscriptionType::Released;
		subscription.Callback.BindSP(Self<Player>(), &Player::OnInput_ESCAPE);
		InputPtr->SubscribeKey(subscription);
		}
		{
			InputPtr->OnMouseMoveDelegate.AddSP(Self<Player>(), &Player::OnMouseMove);
		}
		{
			InputPtr->OnMouseScrollDelegate.AddSP(Self<Player>(), &Player::OnMouseScroll);
		}
	}

	CameraComponent = NewObject<Camera>("PlayerCamera");

	return true;
}

void Player::Tick(float DeltaTime)
{
	SetLocation(GetLocation() + Velocity * MoveSpeed * DeltaTime);
	Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Player::OnInput_W(bool ButtonState, float DeltaTime)
{
	AdjustVelocity(GetForwardVector_World());
}
void Player::OnInput_S(bool ButtonState, float DeltaTime)
{
	AdjustVelocity(-GetForwardVector_World());
}
void Player::OnInput_A(bool ButtonState, float DeltaTime)
{
	glm::vec3 right = glm::normalize(glm::cross(Math::upV, GetForwardVector_World()));
	AdjustVelocity(right);
}
void Player::OnInput_D(bool ButtonState, float DeltaTime)
{
	glm::vec3 right = glm::normalize(glm::cross(Math::upV, GetForwardVector_World()));
	AdjustVelocity(-right);
}
void Player::OnInput_E(bool ButtonState, float DeltaTime)
{
	glm::vec3 up = Math::upV;
	AdjustVelocity(up);
}
void Player::OnInput_Q(bool ButtonState, float DeltaTime)
{
	glm::vec3 up = Math::upV;
	AdjustVelocity(-up);
}
void Player::AdjustVelocity(const glm::vec3& DeltaVelocity)
{
	Velocity += (DeltaVelocity * InputMoveSensitivity);
}
void Player::OnInput_ESCAPE(bool ButtonState, float DeltaTime)
{
	EnginePtr->ShutDown();
}
void Player::OnInput_MOUSERIGHT(bool ButtonState, float DeltaTime)
{
	bMouseInput = ButtonState;
}

void Player::OnMouseScroll(const glm::vec2& DeltaScroll, float DeltaTime)
{
	MoveSpeed = glm::clamp(MoveSpeed + InputMouseScrollSensitivity * DeltaScroll.y, MinMoveSpeed, MaxMoveSpeed);
}

void Player::SetMoveSpeed(float NewSpeed)
{
	MoveSpeed = glm::clamp(NewSpeed, MinMoveSpeed, MaxMoveSpeed);
}
void Player::OnMouseMove(const glm::vec2& CurrentPosition, const glm::vec2& DeltaMove, float DeltaTime)
{
	if ( !bMouseInput || glm::all(glm::epsilonEqual(DeltaMove, glm::vec2(0.0f), FLT_EPS)) ) return;

	glm::quat currentRotation = GetRotation_World();
		
	float yawDelta = glm::radians(DeltaMove.x * -InputMouseMoveSensitivity);
	glm::quat yawRotation = glm::angleAxis(yawDelta, Math::upV);
	
	float pitchDelta = glm::radians(DeltaMove.y * -InputMouseMoveSensitivity);
	glm::quat pitchRotation = glm::normalize(glm::angleAxis(pitchDelta, GetRightVector_World()));

	glm::quat newRotation = glm::normalize(pitchRotation * yawRotation * currentRotation);

	const glm::vec3 newCameraDirection = Math::QuaternionToDirection(newRotation);
	const float MaxPitch = 89.0f;
	const float maxDot = glm::abs(glm::cos(glm::radians(MaxPitch)));
	const float newDot = 1.0f - glm::abs(glm::dot(newCameraDirection, Math::upV));

	if ( newDot <= maxDot )
	{
		const glm::vec3 currentCameraDirection = Math::QuaternionToDirection(currentRotation);
		const float currentDot = 1.0f - glm::abs(glm::dot(currentCameraDirection, Math::upV));

		if ( newDot <= currentDot )
		{
			newRotation = glm::normalize(yawRotation * currentRotation);
		}
	}
	SetRotation(newRotation);
}