
add_executable(vulkan_renderer
	main.c
	math.c

	resource_system.h
	resource_system.c

	rendering/renderer.h
	rendering/renderer.c

	rendering/vulkan_renderer/vulkan_types.h
	rendering/vulkan_renderer/vulkan_renderer.h
	rendering/vulkan_renderer/vulkan_renderer.c
	rendering/vulkan_renderer/vulkan_device.h
	rendering/vulkan_renderer/vulkan_device.c
	rendering/vulkan_renderer/vulkan_swapchain.h
	rendering/vulkan_renderer/vulkan_swapchain.c
	rendering/vulkan_renderer/vulkan_image.h
	rendering/vulkan_renderer/vulkan_image.c
	rendering/vulkan_renderer/vulkan_renderpass.h
	rendering/vulkan_renderer/vulkan_renderpass.c
	rendering/vulkan_renderer/vulkan_pipeline.h
	rendering/vulkan_renderer/vulkan_pipeline.c
	rendering/vulkan_renderer/vulkan_command_buffer.h
	rendering/vulkan_renderer/vulkan_command_buffer.c
	rendering/vulkan_renderer/vulkan_framebuffer.h
	rendering/vulkan_renderer/vulkan_framebuffer.c
	rendering/vulkan_renderer/vulkan_fence.h
	rendering/vulkan_renderer/vulkan_fence.c
)

target_include_directories(vulkan_renderer PRIVATE "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(vulkan_renderer PRIVATE platform)

find_package(Vulkan REQUIRED)
target_include_directories(vulkan_renderer PRIVATE Vulkan::Vulkan)
target_link_libraries(vulkan_renderer PRIVATE Vulkan::Vulkan)

find_library(spirv SPIRV REQUIRED)
find_library(glslang glslang REQUIRED)
find_library(MachineIndependent MachineIndependent REQUIRED)
find_library(OSDependent OSDependent REQUIRED)
find_library(GenericCodeGen GenericCodeGen REQUIRED)
find_library(OGLCompiler OGLCompiler REQUIRED)

target_link_libraries(vulkan_renderer PRIVATE stdc++)
target_link_libraries(vulkan_renderer PRIVATE m)
target_link_libraries(vulkan_renderer PRIVATE ${spirv})
target_link_libraries(vulkan_renderer PRIVATE ${glslang})
target_link_libraries(vulkan_renderer PRIVATE ${MachineIndependent})
target_link_libraries(vulkan_renderer PRIVATE ${OGLCompiler})
target_link_libraries(vulkan_renderer PRIVATE ${OSDependent})
target_link_libraries(vulkan_renderer PRIVATE ${GenericCodeGen})

target_include_directories(vulkan_renderer PRIVATE
	"${PROJECT_SOURCE_DIR}/external/math"
)