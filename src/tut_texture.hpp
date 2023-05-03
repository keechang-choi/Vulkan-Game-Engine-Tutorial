#pragma once

#include <lve_device.hpp>

namespace tut {

class TutImage {
 public:
  TutImage(lve::LveDevice& device, uint32_t width, uint32_t height,
           VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
           VkMemoryPropertyFlags properties);
  ~TutImage();

  TutImage(const TutImage&) = delete;
  TutImage& operator=(const TutImage&) = delete;
  VkImage getImage() const { return image; }

 private:
  lve::LveDevice& lveDevice;
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory imageMemory = VK_NULL_HANDLE;
  void* mapped;
};

class TutTexture {
 public:
  TutTexture(lve::LveDevice& device);
  ~TutTexture();

  TutTexture(const TutImage&) = delete;
  TutTexture& operator=(const TutTexture&) = delete;

  VkSampler getTextureSampler() { return textureSampler; }

 private:
  lve::LveDevice& lveDevice;
  VkSampler textureSampler;
  void createTextureSampler();
};
}  // namespace tut