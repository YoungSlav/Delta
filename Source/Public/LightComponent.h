#pragma once
#include "stdafx.h"
#include "ITransformable.h"
#include "ActorComponent.h"


namespace Delta
{


class LightComponent : public ActorComponent
{
public:
	template <typename... Args>
	LightComponent(Args&&... args) :
		ActorComponent(std::forward<Args>(args)...)
	{}

    void setColor(const glm::vec3& color);
    const glm::vec3& getColor() const;

protected:

private:

    glm::vec3 Color = glm::vec3(1.0f);
};

}