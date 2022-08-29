# Vulkan-Game-Engine-Tutorial
Youtube lecture practice

all the base codes and materials from
- lecture  
  https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR
- github repo  
  https://github.com/blurrypiano/littleVulkanEngine

## Summary

- ~Lec08
  - covered: 
    - env setup
    - window, glfw, glm, RAII
    - pipeline, shader, glsl
    - device, validatioin layer, 
    - pipeline stage configuration, pipeline layout, render pass
    - frame buffer, v-sync, swap chain
    - command buffer, recording
    - vertex buffer, binding, attribute
    - interpolation
    - swap chain recreation, dynamic viewport and scissors
  - ![vulkan-tut-08-01](https://user-images.githubusercontent.com/49244613/186187638-69e4ce50-3d9a-460d-852f-771461ee400d.gif)

- Lec09
  - covered:
    - push constants
    - shader alignment requirement
  - ![vulkan-tut-09-01](https://user-images.githubusercontent.com/49244613/186207249-9b349e49-7fea-40f7-91e8-e8aa5e98dc31.gif)

- Lec10
  - covered:
    - transformation
    - ECS(Entity Component System) (game objects)
  - TODO
    - https://austinmorlan.com/posts/entity_component_system/
  - ![vulkan-tut-10-01](https://user-images.githubusercontent.com/49244613/186472676-8e0779d6-2d12-4e85-832b-3dccda674a01.gif)

- Lec11 
  - covered: 
    - refactoring -> rederer/ render system
    - reder pass compatibility
    - break one-to-one correspondence between swap chain image and command buffer.
  - bonus: gravity simulation
  ![vulkan-tut-11-01](https://user-images.githubusercontent.com/49244613/187266544-9186d8c7-866a-4793-8ba9-f5320463d9ab.gif)
