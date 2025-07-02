#pragma once
#include "stdafx.h"

#include "Object.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Delta
{



class Window : public Object
{
	typedef MulticastDelegate<const glm::ivec2&> OnResizeSignature;

	typedef MulticastDelegate<int32, int32> OnInputKeySignature;
	typedef MulticastDelegate<glm::vec2> OnMouseMoveSignature;
	typedef MulticastDelegate<glm::vec2> OnMouseScrollSignature;

public:
	template <typename... Args>
	Window(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	inline const glm::ivec2& GetViewportSize() const { return ViewportSize; }
	
	GLFWwindow* GetWindow() const { return glfwWindow; }

	void SetMouseEnabled(bool bNewMouseEnabled) const;

	OnResizeSignature OnResizeDelegate;

	OnInputKeySignature OnInputKeyDelegate;
	OnMouseMoveSignature OnMouseMoveDelegate;
	OnMouseScrollSignature OnMouseScrollDelegate;

protected:
	bool Initialize_Internal() override;
	virtual void OnDestroy() override;

private:
	static void Framebuffer_size_callback(struct GLFWwindow* window, int32 width, int32 height);
	
	static void SCallbackMouseScroll(struct GLFWwindow* window, double xoffset, double yoffset);
	static void SCallbackMousePosition(struct GLFWwindow* window, double xpos, double ypos);
	static void SCallbackMouseButton(GLFWwindow* window, int button, int action, int mods);
	static void SCallbackKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

	void Resize(const glm::ivec2& InViewportSize);

private:

	GLFWwindow* glfwWindow;

	glm::ivec2 ViewportSize = glm::vec2(1440.0f, 900.0f);

};


}