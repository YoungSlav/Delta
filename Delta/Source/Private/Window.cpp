#include "stdafx.h"
#include "Window.h"
#include "Engine.h"

using namespace Delta;

bool Window::Initialize_Internal()
{
	Object::Initialize_Internal();

	LOG(Log, "Creating glfw window");

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwWindow = glfwCreateWindow(ViewportSize.x, ViewportSize.y, "Delta app", nullptr, nullptr);
	if ( !glfwWindow )
	{
		glfwTerminate();

		throw std::runtime_error("Failed to create glfw window!");

		return false;
	}

	glfwFocusWindow(glfwWindow);

    glfwSetFramebufferSizeCallback(glfwWindow, Window::Framebuffer_size_callback);

	glfwSetMouseButtonCallback(glfwWindow, Window::SCallbackMouseButton);
	glfwSetCursorPosCallback(glfwWindow, Window::SCallbackMousePosition);
	glfwSetScrollCallback(glfwWindow, Window::SCallbackMouseScroll);
	glfwSetKeyCallback(glfwWindow, Window::SCallbackKeyboard);

	return true;
}

void Window::SetMouseEnabled(bool bNewMouseEnabled) const
{
	LOG(Log, "Mouse enabled {}", bNewMouseEnabled);

	if (bNewMouseEnabled)
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::Framebuffer_size_callback(struct GLFWwindow* window, int32 width, int32 height)
{
	Engine::GetEngine()->GetWindow()->Resize(glm::ivec2(width, height));
}

void Window::Resize(const glm::ivec2& InViewportSize)
{
	OnResizeDelegate.Broadcast(ViewportSize);
}


void Window::SCallbackMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	Engine::GetEngine()->GetWindow()->OnMouseScrollDelegate.Broadcast(glm::vec2(xoffset, yoffset));
}

void Window::SCallbackMousePosition(GLFWwindow* window, double xpos, double ypos)
{
	Engine::GetEngine()->GetWindow()->OnMouseMoveDelegate.Broadcast(glm::vec2(xpos, ypos));
}

void Window::SCallbackMouseButton(GLFWwindow* window, int32 key, int32 action, int32 mods)
{
	Engine::GetEngine()->GetWindow()->OnInputKeyDelegate.Broadcast(key, action);
}

void Window::SCallbackKeyboard(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mods)
{
	Engine::GetEngine()->GetWindow()->OnInputKeyDelegate.Broadcast(key, action);
}

void Window::OnDestroy()
{
	Object::OnDestroy();

	LOG(Log, "Terminating glfw window");

	glfwDestroyWindow(glfwWindow);

	glfwTerminate();
}
