#include "first_app.hpp"

#include "kc_custom.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace lve {

FirstApp::FirstApp() { loadGameObjects(); }
FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{lveDevice,
                                        lveRenderer.getSwapChainRenderPass()};
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();
    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // NOTE: separate frame and renderpass, since we need to control
      // multiple render passes.
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }
  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::vector<LveModel::Vertex> vertices{{{0.0f, -0.6f}, {1.0f, 0.0f, 0.0f}},
                                         {{0.5f, 0.3f}, {0.0f, 1.0f, 0.0f}},
                                         {{-0.5f, 0.3f}, {0.0f, 0.0f, 1.0f}}};
  std::vector<LveModel::Vertex> sierpinski_vertices =
      kc_custom::generate_sierpinski(2, vertices);
  // NOTE(kc): a model can be shared by multiple game objects.
  auto lveModel = std::make_shared<LveModel>(lveDevice, sierpinski_vertices);

  // https://www.color-hex.com/color-palette/5361
  std::vector<glm::vec3> colors{{1.f, .7f, .73f}, {1.f, .87f, .73f},
                                {1.f, 1.f, .73f}, {.73f, 1.f, .8f},
                                {.73, .88f, 1.f}, {1.f, 1.f, 1.f}};
  for (auto& color : colors) {
    color = glm::pow(color, glm::vec3{4.0f});
    std::cout << color[0] << " " << color[1] << " " << color[2] << std::endl;
  }

  const int n = 60;
  for (int i = 0; i < n; i++) {
    auto triangle = LveGameObject::createGameObject();
    triangle.model = lveModel;
    triangle.color = colors[i % colors.size()];
    triangle.transform2d.scale = glm::vec2(.5f) + i / static_cast<float>(n);
    triangle.transform2d.rotation = i * .025f * glm::pi<float>();

    gameObjects.push_back(std::move(triangle));
  }
}

}  // namespace lve
