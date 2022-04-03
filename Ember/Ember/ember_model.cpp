#include <cassert>
#include <cstring>

#include "ember_model.hpp"

namespace ember {

	EmberModel::EmberModel(EmberDevice& device, const std::vector<Vertex>& vertices) : m_emberDevice{device} {
		createVertexBuffers(vertices);
	}

	EmberModel::~EmberModel() {
		vkDestroyBuffer(m_emberDevice.device(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_emberDevice.device(), m_vertexBufferMemory, nullptr);
	}

	void EmberModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

		m_emberDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory
		);

		void *data;
		vkMapMemory(m_emberDevice.device(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_emberDevice.device(), m_vertexBufferMemory);
	}

	void EmberModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void EmberModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	// Vertex struct

	std::vector<VkVertexInputBindingDescription> EmberModel::Vertex::getBindingDescriptions()
	{
		// Binding info for position and color
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> EmberModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		// Vec2 position info
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		// Vec3 color info
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

}