#pragma once

#define	GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>

namespace ember {

	class EmberWindow {
	public:
		EmberWindow(std::string name, int width, int height);
		~EmberWindow();

		EmberWindow(const EmberWindow&) = delete;
		EmberWindow& operator=(const EmberWindow&) = delete;

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		bool shouldClose() { return glfwWindowShouldClose(m_window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };  }
		bool wasWindowResized() { return m_frameBufferResized; }
		void resetWindowResizedFlag() { m_frameBufferResized = false; }

	private:
		const std::string m_name;
		GLFWwindow* m_window;
		int m_width;
		int m_height;
		bool m_frameBufferResized = false;

		void initWindow();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};

}