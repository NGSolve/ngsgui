#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
VkDevice device;

// Find device memory that is supported by the requirements (typeBits) and meets the desired properties
VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t* typeIndex)
{
  for (uint32_t i = 0; i < 32; i++) {
    if ((typeBits & 1) == 1) {
      if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        *typeIndex = i;
        return true;
      }
    }
    typeBits >>= 1;
  }
  return false;
}


template <typename T>
struct UniformBuffer
{
  uint32_t m_binding;
  VkBuffer m_buffer;
  VkDeviceMemory m_memory;
  VkDescriptorBufferInfo m_descriptorBufferInfo = {};
  VkWriteDescriptorSet m_writeDescriptorSet = {};

  UniformBuffer() = delete;
  UniformBuffer(uint32_t binding)
    : m_binding(binding)
  {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(T);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, m_buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex);

    vkAllocateMemory(device, &allocInfo, nullptr, &m_memory);
    vkBindBufferMemory(device, m_buffer, m_memory, 0);

    m_descriptorBufferInfo.buffer = m_buffer;
    m_descriptorBufferInfo.offset = 0;
    m_descriptorBufferInfo.range = sizeof(T);

    m_writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    m_writeDescriptorSet.descriptorCount = 1;
    m_writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_writeDescriptorSet.pBufferInfo = &m_descriptorBufferInfo;
    m_writeDescriptorSet.dstBinding = m_binding;
  }


  ~UniformBuffer()
  {
    vkDestroyBuffer(device, m_buffer, nullptr);
    vkFreeMemory(device, m_memory, nullptr);
  }

  void Update(T & t)
  {
    void* data;
    vkMapMemory(device, m_memory, 0, sizeof(T), 0, &data);
    memcpy(data, &t, sizeof(T));
    vkUnmapMemory(device, m_memory);
  }

};
