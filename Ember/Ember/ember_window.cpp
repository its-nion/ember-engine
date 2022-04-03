#include <stdexcept>

#include "ember_window.hpp"

namespace ember {

	EmberWindow::EmberWindow(std::string name, int width, int height) : m_width{ width }, m_height{ height }, m_name{ name } {
		initWindow();
	}

	EmberWindow::~EmberWindow() {
		glfwDestroyWindow(m_window);
		glfwTerminate;
	}

	void EmberWindow::initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Dont initialize window with OpenGL
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}

	void EmberWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void EmberWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto emberWindow = reinterpret_cast<EmberWindow*>(glfwGetWindowUserPointer(window));
		emberWindow->m_frameBufferResized = true;
		emberWindow->m_width = width;
		emberWindow->m_height = height;
	}

}