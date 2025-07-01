#include "IDeltaRenderable.h"

IDeltaRenderable::IDeltaRenderable(ERenderPriority _RenderPriority) :
	RenderPriority(_RenderPriority),
	RenderGroup(ERenderGroup::Main)
{}

IDeltaRenderable::IDeltaRenderable(ERenderPriority _RenderPriority, ERenderGroup _RenderGroup) :
	RenderPriority(_RenderPriority),
	RenderGroup(_RenderGroup)
{}


ERenderPriority IDeltaRenderable::GetRenderPriority() const { return RenderPriority; }
ERenderGroup IDeltaRenderable::GetRenderGroup() const { return RenderGroup; }

void IDeltaRenderable::SetVisisble(bool bNewVisible)
{
    bVisible = bNewVisible;
}
bool IDeltaRenderable::IsVisisble() const { return bVisible; }

void IDeltaRenderable::SetCastShadows(bool _bCastShadows)
{
    bCastShadows = _bCastShadows;
}
bool IDeltaRenderable::GetCastShadows() const { return bCastShadows; }

void IDeltaRenderable::ObjectClicked(int32 InstanceIndex)
{
	OnObjectClicked.Broadcast(InstanceIndex);
};