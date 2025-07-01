#include "IRenderable.h"

using namespace Delta;

IRenderable::IRenderable(ERenderPriority _RenderPriority) :
	RenderPriority(_RenderPriority),
	RenderGroup(ERenderGroup::Main)
{}

IRenderable::IRenderable(ERenderPriority _RenderPriority, ERenderGroup _RenderGroup) :
	RenderPriority(_RenderPriority),
	RenderGroup(_RenderGroup)
{}


ERenderPriority IRenderable::GetRenderPriority() const { return RenderPriority; }
ERenderGroup IRenderable::GetRenderGroup() const { return RenderGroup; }

void IRenderable::SetVisisble(bool bNewVisible)
{
    bVisible = bNewVisible;
}
bool IRenderable::IsVisisble() const { return bVisible; }

void IRenderable::SetCastShadows(bool _bCastShadows)
{
    bCastShadows = _bCastShadows;
}
bool IRenderable::GetCastShadows() const { return bCastShadows; }

void IRenderable::ObjectClicked(int32 InstanceIndex)
{
	OnObjectClicked.Broadcast(InstanceIndex);
};