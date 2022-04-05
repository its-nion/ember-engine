#include <stdexcept>
#include <array>

#include "ember_app.hpp"

namespace ember {

	EmberApp::EmberApp() {
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	EmberApp::~EmberApp() {
		vkDestroyPipelineLayout(m_emberDevice.device(), m_pipelineLayout, nullptr);
	}

	void EmberApp::run() {
		while (!m_emberWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(m_emberDevice.device());
	}

	void sierpinski(std::vector<EmberModel::Vertex>& vertices, int depth, glm::vec2 top, glm::vec2 right, glm::vec2 left) {
		if (depth <= 0) {
			vertices.push_back({ top , {1.0f, 1.0f, 1.0f} });
			vertices.push_back({ right , {1.0f, 1.0f, 1.0f} });
			vertices.push_back({ left, {1.0f, 1.0f, 1.0f} });
		}
		else {
			auto leftMidPoint = top + 0.5f * (left - top);
			auto rightMidPoint = top + 0.5f * (right - top);
			auto bottomMidPoint = left + 0.5f * (right - left);

			sierpinski(vertices, depth - 1, top, rightMidPoint, leftMidPoint); // Top triangle
			sierpinski(vertices, depth - 1, rightMidPoint, right, bottomMidPoint); // Right triangle
			sierpinski(vertices, depth - 1, leftMidPoint, bottomMidPoint, left); // Left triangle
		}
	}

	void EmberApp::loadModels() {
		/*std::vector<EmberModel::Vertex> vertices;
		sierpinski(vertices, 5, { 0.0f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f });*/

		std::vector<EmberModel::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		m_emberModel = std::make_unique<EmberModel>(m_emberDevice, vertices);
	}

	void EmberApp::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_emberDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void EmberApp::createPipeline() {
		assert(m_emberSwapChain != nullptr && "Cannot create pipeline before swapchain!");
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");


		PipelineConfigInfo pipelineConfig{};
		EmberPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = m_emberSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_emberPipeline = std::make_unique<EmberPipeline>(
			m_emberDevice,
			"E:\\Work\\Engine\\Ember\\Ember\\Shader\\simple_shader.vert.spv",
			"E:\\Work\\Engine\\Ember\\Ember\\Shader\\simple_shader.frag.spv",
			pipelineConfig
			);
	}

	void EmberApp::recreateSwapChain() {
		auto extent = m_emberWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = m_emberWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_emberDevice.device());
		//m_emberSwapChain = nullptr;

		if (m_emberSwapChain == nullptr) {
			m_emberSwapChain = std::make_unique<EmberSwapChain>(m_emberDevice, extent);
		} else {
			m_emberSwapChain = std::make_unique<EmberSwapChain>(m_emberDevice, extent, std::move(m_emberSwapChain));

			if (m_emberSwapChain->imageCount() != m_commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		createPipeline();
	}

	void EmberApp::createCommandBuffers() {
		m_commandBuffers.resize(m_emberSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_emberDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_emberDevice.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers");
		}
	}

	void EmberApp::freeCommandBuffers() {
		vkFreeCommandBuffers(m_emberDevice.device(), m_emberDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
		m_commandBuffers.clear();
	}

	void EmberApp::recordCommandBuffer(int imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording the command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_emberSwapChain->getRenderPass();
		renderPassInfo.framebuffer = m_emberSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_emberSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_emberSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_emberSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_emberSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

		m_emberPipeline->bind(m_commandBuffers[imageIndex]);
		m_emberModel->bind(m_commandBuffers[imageIndex]);
		m_emberModel->draw(m_commandBuffers[imageIndex]);

		vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}
	}

	void EmberApp::drawFrame() {
		uint32_t imageIndex;
		auto result = m_emberSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image");
		}

		recordCommandBuffer(imageIndex);

		result = m_emberSwapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_emberWindow.wasWindowResized()) {
			m_emberWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image");
		}
	}
}