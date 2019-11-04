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


// template <typename T>
// struct UniformBuffer
// {
//   uint32_t m_binding;
//   VkBuffer m_buffer;
//   VkDeviceMemory m_memory;
//   VkDescriptorBufferInfo m_descriptorBufferInfo = {};
//   VkWriteDescriptorSet m_writeDescriptorSet = {};
// 
//   UniformBuffer() = delete;
//   UniformBuffer(uint32_t binding)
//     : m_binding(binding)
//   {
//     VkBufferCreateInfo bufferInfo = {};
//     bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//     bufferInfo.size = sizeof(T);
//     bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
// 
//     vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer);
// 
//     VkMemoryRequirements memReqs;
//     vkGetBufferMemoryRequirements(device, m_buffer, &memReqs);
// 
//     VkMemoryAllocateInfo allocInfo = {};
//     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     allocInfo.allocationSize = memReqs.size;
//     getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex);
// 
//     vkAllocateMemory(device, &allocInfo, nullptr, &m_memory);
//     vkBindBufferMemory(device, m_buffer, m_memory, 0);
// 
//     m_descriptorBufferInfo.buffer = m_buffer;
//     m_descriptorBufferInfo.offset = 0;
//     m_descriptorBufferInfo.range = sizeof(T);
// 
//     m_writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     m_writeDescriptorSet.descriptorCount = 1;
//     m_writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     m_writeDescriptorSet.pBufferInfo = &m_descriptorBufferInfo;
//     m_writeDescriptorSet.dstBinding = m_binding;
// 
//     m_writeDescriptorSet.dstArrayElement = 0;
//     m_writeDescriptorSet.descriptorCount = 1;
//   }
// 
// 
//   ~UniformBuffer()
//   {
//     vkDestroyBuffer(device, m_buffer, nullptr);
//     vkFreeMemory(device, m_memory, nullptr);
//   }
// 
//   void Update(T & t)
//   {
//     void* data;
//     vkMapMemory(device, m_memory, 0, sizeof(T), 0, &data);
//     memcpy(data, &t, sizeof(T));
//     vkUnmapMemory(device, m_memory);
//   }
// };



struct GlobalSettings
{
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  VkDescriptorSet descriptorSet;

  uint32_t m_binding = 0;
  VkBuffer m_buffer;
  VkDeviceMemory m_memory;
  VkDescriptorBufferInfo m_descriptorBufferInfo = {};
  VkWriteDescriptorSet m_writeDescriptorSet = {};

  struct UniformBuffer
  {
    glm::mat4 MVP;
    glm::mat4 MV;
    glm::mat4 MV_inv_trans;

    glm::vec3 light_dir;
    float light_ambient;
    float light_diffuse;
    float light_spec;
    float light_shininess;

    int mesh_dim;
    int mesh_offset;

    float colormap_min;
    float colormap_max;
    int colormap_n;
    bool colormap_linear;
  };

  UniformBuffer uniforms;

  GlobalSettings()
  {}

  void createDescriptorPool() {
    // This describes how many descriptor sets we'll create from this pool for each type
    VkDescriptorPoolSize typeCount;
    typeCount.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCount.descriptorCount = 1;

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = 1;
    createInfo.pPoolSizes = &typeCount;
    createInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
      std::cerr << "failed to create descriptor pool" << std::endl;
      exit(1);
    } else {
      std::cout << "created descriptor pool" << std::endl;
    }

  }

  void createUniformBuffer()
  {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(UniformBuffer);
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
    m_descriptorBufferInfo.range = sizeof(UniformBuffer);

    m_writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    m_writeDescriptorSet.descriptorCount = 1;
    m_writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_writeDescriptorSet.pBufferInfo = &m_descriptorBufferInfo;
    m_writeDescriptorSet.dstBinding = m_binding;

    m_writeDescriptorSet.dstArrayElement = 0;
    m_writeDescriptorSet.descriptorCount = 1;
  }

  void Init( VkPipelineLayout & pipelineLayout )
  {
    // Describe pipeline layout
    // Note: this describes the mapping between memory and shader resources (descriptor sets)
    // This is for uniform buffers and samplers
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo = {};
    descriptorLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutCreateInfo.bindingCount = 1;
    descriptorLayoutCreateInfo.pBindings = &layoutBinding;

    if (vkCreateDescriptorSetLayout(device, &descriptorLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
      std::cerr << "failed to create descriptor layout" << std::endl;
      exit(1);
    } else {
      std::cout << "created descriptor layout" << std::endl;
    }

    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 1;
    layoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
      std::cerr << "failed to create pipeline layout" << std::endl;
      exit(1);
    } else {
      std::cout << "created pipeline layout" << std::endl;
    }
  }

  ~GlobalSettings()
  {
    vkDestroyBuffer(device, m_buffer, nullptr);
    vkFreeMemory(device, m_memory, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  }

  void createDescriptorSet() {
    // There needs to be one descriptor set per binding point in the shader
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
      std::cerr << "failed to create descriptor set" << std::endl;
      exit(1);
    } else {
      std::cout << "created descriptor set" << std::endl;
    }

    auto writeDescriptorSet = m_writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSet;
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
  }

  void updateUniformData( VkExtent2D & swapChainExtent )
  {
    // Rotate based on time
    static std::chrono::high_resolution_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
    auto timeNow = std::chrono::high_resolution_clock::now();
    long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeStart - timeNow).count();
    float angle = (millis % 4000) / 4000.0f * glm::radians(360.f);

    glm::mat4 modelMatrix{};
    for (int i=0;i<4;i++)
      modelMatrix[i][i] = 1.0;
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 0, 1));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f / 3.0f, -0.5f / 3.0f, 0.0f));
    float s = 0.3f;
    modelMatrix = glm::scale(modelMatrix, {s,s,s});

    // Set up view
    auto viewMatrix = glm::lookAt(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));

    // Set up projection
    auto projMatrix = glm::perspective(glm::radians(70.f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);

    auto MV = viewMatrix * modelMatrix;

    uniforms.MVP = projMatrix * MV;
    uniforms.MV = MV;
    uniforms.MV_inv_trans = glm::inverse(glm::transpose(MV));

    uniforms.light_dir[0] = 1.0f;
    uniforms.light_dir[1] = 3.0f;
    uniforms.light_dir[2] = 3.0f;
    uniforms.light_ambient = 0.3f;
    uniforms.light_diffuse = 0.7f;
    uniforms.light_spec = 0.0f;
    uniforms.light_shininess = 1.0f;

    uniforms.mesh_dim = 3;
    uniforms.mesh_offset = 0;

    void* data;
    vkMapMemory(device, m_memory, 0, sizeof(UniformBuffer), 0, &data);
    memcpy(data, &uniforms, sizeof(UniformBuffer));
    vkUnmapMemory(device, m_memory);
  }


};
