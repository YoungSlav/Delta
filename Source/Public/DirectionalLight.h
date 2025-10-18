#pragma once

#include "stdafx.h"
#include "LightComponent.h"


namespace Delta
{


class DirectionalLight : public LightComponent
{
public:
	template <typename... Args>
	DirectionalLight(Args&&... args) :
		LightComponent(std::forward<Args>(args)...)
	{}
};

}