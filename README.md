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


