
add_executable(vulkan_renderer
	main.c

	rendering/renderer.h
	rendering/renderer.c
	rendering/renderer_internal.h

	rendering/vulkan_renderer/vulkan_context.h
	rendering/vulkan_renderer/vulkan_renderer.h
	rendering/vulkan_renderer/vulkan_renderer.c
	rendering/vulkan_renderer/vulkan_internal.h
	rendering/vulkan_renderer/vulkan_device.c
	rendering/vulkan_renderer/vulkan_swapchain.c
	
	rendering/directx_renderer/directx_context.h
)

target_include_directories(vulkan_renderer PRIVATE "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(vulkan_renderer PRIVATE platform)

find_package(Vulkan REQUIRED)
target_include_directories(vulkan_renderer PRIVATE Vulkan::Vulkan)
target_link_libraries(vulkan_renderer PRIVATE Vulkan::Vulkan)