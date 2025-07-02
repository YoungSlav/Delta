#pragma once
#include "stdafx.h"

namespace Delta
{

enum ERenderPriority : int32
{
	Early = 0,
	Starndart = 1,
	Late = 2
};

enum ERenderGroup : int32
{
		Main = 0,
		Overlay = 1
};

typedef MulticastDelegate<const int32&> OnObjectClickedSignature;

class IRenderable
{

public:
	IRenderable() = default;

	IRenderable(ERenderPriority _RenderPriority);

	IRenderable(ERenderPriority _RenderPriority, ERenderGroup _RenderGroup);

	virtual ~IRenderable() = default;

	ERenderPriority GetRenderPriority() const;
	ERenderGroup GetRenderGroup() const;

	void SetVisisble(bool bNewVisible);
	virtual bool IsVisisble() const;

	void SetCastShadows(bool _bCastShadows);
	bool GetCastShadows() const;


	virtual void Render() = 0;
	virtual void RenderDepthMap(std::shared_ptr<class DeltaShaderAsset> Shader) {}
	
	// selection
	virtual void RenderSelectionID() {};
	virtual void RenderOutlineMask() {};

	OnObjectClickedSignature OnObjectClicked;

	void ObjectClicked(int32 InstanceIndex = 0);

protected:
	const ERenderPriority RenderPriority = Starndart;
	const ERenderGroup RenderGroup = Main;
	
private:

	bool bCastShadows = false;
	bool bVisible = true;
	
};

}