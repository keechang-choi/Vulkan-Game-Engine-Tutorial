#pragma once

#include "lve_buffer.hpp"
#include "lve_device.hpp"
#include "tut_texture.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace lve {
class LveModel {
 public:
  struct Vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return position == other.position && color == other.color &&
             normal == other.normal && uv == other.uv;
    }
  };

  struct Builder {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    std::string texture_path;

    void loadModel(const std::string &filepath);
  };

  LveModel(LveDevice &device, const LveModel::Builder &builder);
  ~LveModel();

  LveModel(const LveModel &) = delete;
  LveModel &operator=(const LveModel &) = delete;

  static std::unique_ptr<LveModel> createModelFromFile(
      LveDevice &device, const std::string &filepath,
      const std::string &texture_path);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

  // return raw pointer of texture image instance.
  // if no texture image exists, return nullptr.
  tut::TutImage *getTextureImagePtr() { return textureImage.get(); }
  VkImageView getTextureImageView() { return textureImageView; }
  VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;

 private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);
  void createTextureImage(const std::string &texture_path);
  void createTextureImageView();

  LveDevice &lveDevice;

  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;

  std::unique_ptr<tut::TutImage> textureImage;
  VkImageView textureImageView = VK_NULL_HANDLE;
};
}  // namespace lve