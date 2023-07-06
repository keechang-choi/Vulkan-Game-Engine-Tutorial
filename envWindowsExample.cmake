# set(GLFW_PATH C:/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/Libraries/glfw-3.3.8)
# set(GLM_PATH C:/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/Libraries/glm)

if ("$ENV{VK_SDK_PATH}" STREQUAL "")
    set(VK_SDK_PATH  C:/VulkanSDK/1.3.216.0)
else()
    set(VK_SDK_PATH "$ENV{VK_SDK_PATH}")
    file(TO_CMAKE_PATH ${VK_SDK_PATH} VK_SDK_PATH)
endif()
message("VK_SDK_PATH = ${VK_SDK_PATH}")

# Set MINGW_PATH if using mingwBuild.bat and not VisualStudio20XX
if ("$ENV{MINGW_PATH}" STREQUAL "")
    set(MINGW_PATH "C:/Program\ Files/mingw-w64/x86_64-12.2.0-release-posix-seh-rt_v10-rev1/mingw64")
else()
    set(MINGW_PATH "$ENV{MINGW_PATH}")
    file(TO_CMAKE_PATH ${MINGW_PATH} MINGW_PATH)
endif()
message("MINGW_PATH = ${MINGW_PATH}")

# Optional set TINYOBJ_PATH to target specific version, otherwise defaults to external/tinyobjloader
# set(TINYOBJ_PATH X:/dev/Libraries/tinyobjloader)