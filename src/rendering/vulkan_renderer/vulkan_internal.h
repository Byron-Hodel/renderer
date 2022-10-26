#ifndef VULKAN_INTERNAL_H
#define VULKAN_INTERNAL_H

#include <vulkan/vulkan.h>
#include <platform/platform.h>
#include "vulkan_context.h"
#include "rendering/renderer.h"

#define GRAPHICS_QUEUE_INDEX    0
#define COMPUTE_QUEUE_INDEX     1
#define TRANSFER_QUEUE_INDEX    2
#define SPARSE_BIND_QUEUE_INDEX 3

struct render_target_t {
	platform_window_t* window;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR swapchain_format;
	VkPresentModeKHR swapchain_present_mode;

	VkImage* swapchain_images;
	VkImageView* swapchain_image_views;
	uint32_t swapchain_image_count;
	VkExtent2D swapchain_extent;

	VkFramebuffer* framebuffers;
	uint32_t framebuffer_count;
};

// defined in vulkan_renderer.c
extern const uint32_t layer_count;
extern const char* layer_names[];
extern const uint32_t device_extension_count;
extern const char* device_extension_names[];

int8_t _vulkan_init_physical_devices(vulkan_context_t* context);
void _vulkan_cleanup_physical_device(vulkan_context_t* context);
int8_t _vulkan_select_physical_device(vulkan_context_t* context);
int8_t _vulkan_init_selected_device(vulkan_context_t* context);
void _vulkan_cleanup_selected_device(vulkan_context_t* context);

void _vulkan_get_swapchain_modes(vulkan_context_t* context, render_target_t* render_target);
int8_t _vulkan_create_swapchain(platform_context_t* platform_context, vulkan_context_t* context, render_target_t* render_target);
void _vulkan_destroy_swapchain(vulkan_context_t* context, render_target_t* render_target);

#endif // VULKAN_INTERNAL_H