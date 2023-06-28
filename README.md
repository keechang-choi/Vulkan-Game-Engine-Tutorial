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
- Lec12
  - covered:
    - homogeneous coordinates
    - Euler angle
    - 3d rendering
  - <details>
    <summary> intrinsic rotation, extrinsic rotation </summary>

    # intrinsic rotation, extrinsic rotation

    [https://math.stackexchange.com/questions/1137745/proof-of-the-extrinsic-to-intrinsic-rotation-transform](https://math.stackexchange.com/questions/1137745/proof-of-the-extrinsic-to-intrinsic-rotation-transform)

    4개의 basis와 변환.  rotation mat들은 모두 similar.

    $$  
    \begin{align}
    P_0 = S_{world_0}^{-1}  \\
    P_0: xyz \rightarrow x_0y_0z_0 \\
    P_1: x_0y_0z_0 \rightarrow  x_1y_1z_1  \\
    P_2: x_1y_1z_1 \rightarrow  x_2y_2z_2  
    \end{align}
    $$  

    intrinsic xyz 순서로 alpha, beta, gamma 회전을 한다는것은 다음 mat연산

    $$
    R = Z''(\gamma) P_2  Y'(\beta) P_1 X(\alpha)
    $$

    $$
    \begin{align}
    X: A_{x_{0} y_{0} z_{0}} \rightarrow A_{x_{0} y_{0} z_{0}} \\
    Y' P_1: A_{x_{0} y_{0} z_{0}} \rightarrow A_{x_{1} y_{1} z_{1}} \\
    Z'' P_2: A_{x_{1} y_{1} z_{1}} \rightarrow A_{x_{2} y_{2} z_{2}}
    \end{align}
    $$


    Claim) : extrinsic으로 볼 수도 있다. (회전 축 순서는 반대방향)

    $$
    \begin{align}
    S_{world_2}Z''(\gamma) P_2 Y'(\beta) P_1 X(\alpha) S_{world_0}^{-1} = X^{\*}(\alpha) Y^{\*}(\beta) Z^{\*}(\gamma) \\   
    where \\; S_{world_2} = (P_0 P_1 P_2)^{-1} \\; and \\; S_{world_0}^{-1} = P_0 \\   
    X^{\*}: A_{xyz} \rightarrow A_{xyz} \\  
    Y^{\*}: A_{xyz} \rightarrow A_{xyz} \\  
    Z^{\*}: A_{xyz} \rightarrow A_{xyz}  
    \end{align}
    $$

    Pf)

    $$
    \begin{align}
    Y^{\*} = S_{world_1} Y' S_{world_1}^{-1} =  (P_1 P_0)^{-1} Y'(P_1 P_0) \\
    Z^{\*} = (P_2 P_1 P_0)^{-1} Z''(P_2 P_1 P_0) \\
    \Rightarrow  X^{\*} Y^{\*} Z^{\*} = S_{world_2} Z''P_2 Y' P_1 X P_0 = \\ 
    P_0^{-1}(P_1^{-1} P_2^{-1} Z''P_2 Y' P_1 X )  P_0
    \end{align}
    $$

    </details>

- Lec13
  - covered:
    - projection
      - orthographic
      - perspective
    - homogeneous coordinates
  - ![tut13](https://user-images.githubusercontent.com/49244613/188308317-a99b93b8-22f4-46ee-ac90-4876177033e9.gif)
- Lec14
  - covered:
    - cameara transformation
    - Viewing transformation  (model - camera - perspective - orthographic - viewport)  
- Lec15
  - covered:
    - game loop and time
    - camera controller
- Lec16
  - covered:
    - index buffer
    - staging buffer
- Lec17
  - covered:
    - wavefront .obj format
    - tiny obj loader
    - .obj format using index buffer (vertex hash)
  - <img width="376" alt="image" src="https://user-images.githubusercontent.com/49244613/193910632-ab7893cc-b17b-4a13-9a6f-4e027e1ec84c.png">
- Lec18
  - covered:
    - lighting model. lambertian
    - directional light, diffuse, ambient
    - smooth shading, flat shading
    - surface normal transform
  - ![vulkan-tut-18](https://user-images.githubusercontent.com/49244613/194142751-f1cfbf39-76bc-49fe-9494-629d3fc2d1ad.gif)
- Lec19
  - covered:
    - uniform buffer, alignment
    - refactoring buffer abstraction, descriptor set intro
    - refactoring frame info
- Lec20
  - covered:
    - Descriptors, descriptor set, descriptor set layout, descriptor pool
    - update projection view model matrix to use uniform buffer.
- Lec21
  - covered:
    - Point light, attenuation
    - floor object
  - ![image](https://user-images.githubusercontent.com/49244613/202700802-8d6f446c-d5ac-4dec-a52e-2770ded9dd8c.png)
- Lec22
  - covered:
    - per fragment lighting.
    - ECS refactoring - game objects controlled by a map with key of ID.
  - ![image](https://user-images.githubusercontent.com/49244613/202851398-abd3cc96-8205-46d8-a731-eaadfd1bff5f.png)
- Lec23
  - covered:
    - build system refactoring with cmake
- Lec24
  - covered:
    - billboard - screen aligned
    - point light render system
  - ![vulkan-tutorial24-2](https://user-images.githubusercontent.com/49244613/202894579-deaaf732-49da-4f0b-8a3c-ba08ec2cbd80.gif)
- Lec25
  - covered:
    - multiple point light 
      - multiple light calculation in shader
      - multiple light source rendering
  - ![vulkan-tut-25](https://user-images.githubusercontent.com/49244613/205503127-a7127bc3-6890-4c0b-a72e-5822f7102cae.gif)
- Lec26
  - covered:
    - specular lighting
    - Blinn-Phong model, inverse view matrix and half angle vector
  - ![image](https://user-images.githubusercontent.com/49244613/205640257-d2796ac6-b9bd-4713-8eee-1aacfa55f4a4.png)
- Lec27
  - covered:
    - alpha blending
    - sorting semi-transparents object in rendering
  - ![vulkan-tut27](https://user-images.githubusercontent.com/49244613/205864135-b482708f-81c9-4b2c-beca-11991455516f.gif)

- Tutorial Texture mapping
  - covered:
    - image, imageView, texture sampler
    - texture resource descriptor set per object
  - ![vulkan-tutorial-texture-2](https://user-images.githubusercontent.com/49244613/234461717-f75fd7c5-2cba-4356-a569-935b57069a85.gif)
- Tutorial Depth buffering
  - covered: 
    - depth image, render pass update
  - ![image](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/e000c2d0-d91a-42b4-96c9-ed898bef7b60)
- Tutorial Loading models
  - covered: 
    - textured model loading, vertex deduplication
  - refacotoring: point light rotation system
  - ![image](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/d345443d-150d-4904-b899-dbaa61ec75cf)
- Tutorial Generating mipmaps
  - covered: 
    - using mipmap, image blit command and transition, sampler mipmap update
  - ![vulkan-study-mipmap](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/564f2603-199c-400e-bca3-2c778a388e31)
  - ![image](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/b1967352-31a6-441e-a181-a0406ee0761f)
  - ![image](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/544c079f-5072-41ea-8c8b-3f1932468fef)
- Tutorial Multi-Sampling-Anti-Aliasing
  - covered:
    - MSAA sample count, render target update, sample shading
  - | x1 w/o MSAA | MSAAx4 |
    | ----------- | ----------- |
    | ![image](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/73ab1727-3a69-461e-aa1c-3edc678c2b63) | ![image](https://github.com/keechang-choi/Vulkan-Game-Engine-Tutorial/assets/49244613/7ca0b3e3-388c-49ec-93da-8e7b80732cd8) |

    

