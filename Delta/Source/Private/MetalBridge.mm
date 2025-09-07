#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

extern "C" void* Delta_GetNSViewFromGLFW(GLFWwindow* window)
{
    NSWindow* nswin = glfwGetCocoaWindow(window);
    if (!nswin) return nullptr;
    NSView* view = [nswin contentView];
    [view setWantsLayer:YES];
    if (![view layer]) {
        CAMetalLayer* layer = [CAMetalLayer layer];
        [view setLayer:layer];
    }
    return (__bridge void*)view;
}
#endif

