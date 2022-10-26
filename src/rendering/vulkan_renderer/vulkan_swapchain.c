#include "vulkan_internal.h"
#include <malloc.h>

void _vulkan_get_swapchain_modes(vulkan_context_t* context, render_target_t* render_target) {
	VkPhysicalDevice physical_device = context->selected_device.physical_device.handle;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, render_target->surface,
	                                          &render_target->surface_capabilities);

	uint32_t present_mode_count = 0;
	VkPresentModeKHR present_modes[64];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, render_target->surface,
	                                          &present_mode_count, NULL);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, render_target->surface,
	                                          &present_mode_count, present_modes);

	uint32_t format_count;
	VkSurfaceFormatKHR formats[64];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, render_target->surface,
	                                     &format_count, NULL);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, render_target->surface,
	                                     &format_count, formats);

	VkPresentModeKHR selected_mode = VK_PRESENT_MODE_FIFO_KHR;
	VkSurfaceFormatKHR selected_format = formats[0];
	for(int i = 0; i < format_count; i++) {
		VkSurfaceFormatKHR format = formats[i];
		if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
		   format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			selected_format = format;
			break;
		}
	}

	render_target->swapchain_present_mode = selected_mode;
	render_target->swapchain_format = selected_format;
}

int8_t _vulkan_create_swapchain(platform_context_t* platform_context, vulkan_context_t* context, render_target_t* render_target) {
	VkPhysicalDevice physical_device = context->selected_device.physical_device.handle;
	VkDevice logical_device = context->selected_device.logical_device;
	
	uint32_t image_count = render_target->surface_capabilities.minImageCount+1;
	uint32_t max_image_count = render_target->surface_capabilities.maxImageCount;
	if(max_image_count > 0 && image_count > max_image_count) image_count = max_image_count;
	
	uint32_t width, height;
	platform_get_window_size(platform_context, render_target->window, &width, &height);
	VkExtent2D extents = { width, height };
	if(extents.width < render_target->surface_capabilities.minImageExtent.width) {
		extents.width = render_target->surface_capabilities.minImageExtent.width;
	}
	else if(extents.width > render_target->surface_capabilities.maxImageExtent.width) {
		extents.width = render_target->surface_capabilities.maxImageExtent.width;
	}
	if(extents.height < render_target->surface_capabilities.minImageExtent.height) {
		extents.height = render_target->surface_capabilities.minImageExtent.height;
	}
	else if(extents.height > render_target->surface_capabilities.maxImageExtent.height) {
		extents.height = render_target->surface_capabilities.maxImageExtent.height;
	}
	render_target->swapchain_extent = extents;

	VkSwapchainCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	create_info.surface = render_target->surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = render_target->swapchain_format.format;
	create_info.imageColorSpace = render_target->swapchain_format.colorSpace;
	create_info.imageExtent = extents;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.preTransform = render_target->surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.clipped = VK_TRUE;

	VkResult swapchain_result = vkCreateSwapchainKHR(logical_device, &create_info, NULL, &render_target->swapchain);
	if(swapchain_result != VK_SUCCESS) return 0;

	vkGetSwapchainImagesKHR(logical_device, render_target->swapchain,
	                        &render_target->swapchain_image_count, NULL);
	render_target->swapchain_images = calloc(sizeof(VkImage), render_target->swapchain_image_count);
	vkGetSwapchainImagesKHR(logical_device, render_target->swapchain,
	                        &render_target->swapchain_image_count, render_target->swapchain_images);

	render_target->swapchain_image_views = calloc(sizeof(VkImageView), render_target->swapchain_image_count);

	VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_create_info.format = render_target->swapchain_format.format,
	view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_create_info.subresourceRange.baseMipLevel = 0;
	view_create_info.subresourceRange.levelCount = 1;
	view_create_info.subresourceRange.baseArrayLayer = 0;
	view_create_info.subresourceRange.layerCount = 1;
	for(int i = 0; i < render_target->swapchain_image_count; i++) {
		view_create_info.image = render_target->swapchain_images[i];
		VkResult r = vkCreateImageView(logical_device, &view_create_info,
		                               NULL, &render_target->swapchain_image_views[i]);
		if(r != VK_SUCCESS) {
			free(render_target->swapchain_image_views);
			free(render_target->swapchain_images);
			vkDestroySwapchainKHR(logical_device, render_target->swapchain, NULL);
		}
	}

	return 1;
}
void _vulkan_destroy_swapchain(vulkan_context_t* context, render_target_t* render_target) {
	free(render_target->swapchain_images);
	free(render_target->swapchain_image_views);
	vkDestroySwapchainKHR(context->selected_device.logical_device,
	                      render_target->swapchain, NULL);
}