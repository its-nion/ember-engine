#pragma once

#include <memory>
#include <vector>

#include "ember_window.hpp"
#include "ember_pipeline.hpp"
#include "ember_device.hpp"
#include "ember_swap_chain.hpp"
#include "ember_model.hpp"

namespace ember {

	class EmberApp {
	public:
		static constexpr int WIDTH = 960;
		static constexpr int HEIGHT = 540;

		EmberApp();
		~EmberApp();

		EmberApp(const EmberApp&) = delete;
		EmberApp& operator=(const EmberApp&) = delete;

		void run();
	private:
		EmberWindow m_emberWindow{ "Ember App", WIDTH, HEIGHT };
		EmberDevice m_emberDevice{ m_emberWindow };
		std::unique_ptr<EmberSwapChain> m_emberSwapChain;
		std::unique_ptr<EmberPipeline> m_emberPipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::unique_ptr<EmberModel> m_emberModel;

		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
	};

}