#pragma once
#include "stdafx.h"

#include "Object.h"


namespace Delta
{



class Window final : public Object
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

	inline const glm::ivec2& getViewportSize() const { return viewportSize; }
	
	GLFWwindow* getWindow() const { return glfwWindow; }

	void setMouseEnabled(bool bNewMouseEnabled) const;

	OnResizeSignature OnResizeDelegate;

	OnInputKeySignature OnInputKeyDelegate;
	OnMouseMoveSignature OnMouseMoveDelegate;
	OnMouseScrollSignature OnMouseScrollDelegate;

protected:
	bool initialize_Internal() override;
	virtual void onDestroy() override;

private:
	static void windowSizeCallback(struct GLFWwindow* window, int32 width, int32 height);
	
	static void windowMouseScrollCallback(struct GLFWwindow* window, double xoffset, double yoffset);
	static void windowMousePositionCallback(struct GLFWwindow* window, double xpos, double ypos);
	static void windowMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void windowKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void resize(const glm::ivec2& InViewportSize);

private:

	GLFWwindow* glfwWindow;

	glm::ivec2 viewportSize = glm::vec2(1440.0f, 900.0f);

};


}