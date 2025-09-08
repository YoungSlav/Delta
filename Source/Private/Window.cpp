#include "stdafx.h"
#include "Window.h"
#include "Engine.h"

using namespace Delta;

bool Window::initialize_Internal()
{
	Object::initialize_Internal();

	LOG(Log, "Creating glfw window");

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwWindow = glfwCreateWindow(viewportSize.x, viewportSize.y, "Delta app", nullptr, nullptr);
	if ( !glfwWindow )
	{
		glfwTerminate();

		throw std::runtime_error("Failed to create glfw window!");

		return false;
	}

	glfwFocusWindow(glfwWindow);

    glfwSetFramebufferSizeCallback(glfwWindow, Window::windowSizeCallback);

	glfwSetMouseButtonCallback(glfwWindow, Window::windowMouseButtonCallback);
	glfwSetCursorPosCallback(glfwWindow, Window::windowMousePositionCallback);
	glfwSetScrollCallback(glfwWindow, Window::windowMouseScrollCallback);
	glfwSetKeyCallback(glfwWindow, Window::windowKeyboardCallback);

	return true;
}

void Window::setMouseEnabled(bool bNewMouseEnabled) const
{
	LOG(Log, "Mouse enabled {}", bNewMouseEnabled);

	if (bNewMouseEnabled)
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::windowSizeCallback(struct GLFWwindow* window, int32 width, int32 height)
{
	Engine::getEngine()->getWindow()->resize(glm::ivec2(width, height));
}

void Window::resize(const glm::ivec2& InViewportSize)
{
	OnResizeDelegate.Broadcast(viewportSize);
}


void Window::windowMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Engine::getEngine()->getWindow()->OnMouseScrollDelegate.Broadcast(glm::vec2(xoffset, yoffset));
}

void Window::windowMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	Engine::getEngine()->getWindow()->OnMouseMoveDelegate.Broadcast(glm::vec2(xpos, ypos));
}

void Window::windowMouseButtonCallback(GLFWwindow* window, int32 key, int32 action, int32 mods)
{
	Engine::getEngine()->getWindow()->OnInputKeyDelegate.Broadcast(key, action);
}

void Window::windowKeyboardCallback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mods)
{
	Engine::getEngine()->getWindow()->OnInputKeyDelegate.Broadcast(key, action);
}

void Window::onDestroy()
{
	Object::onDestroy();

	LOG(Log, "Terminating glfw window");

	glfwDestroyWindow(glfwWindow);

	glfwTerminate();
}
