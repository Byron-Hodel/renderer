#include "vulkan_swapchain.h"
#include "vulkan_image.h"
#include "vulkan_fence.h"
#include "vulkan_framebuffer.h"
#include <malloc.h>

int8_t vulkan_swapchain_create(const vulkan_context_t context, platform_window_t* window, vulkan_swapchain_t* swapchain) {
	vulkan_swapchain_t sc;
	sc.surface = platform_vulkan_create_surface(window, context.instance);
	if(sc.surface == VK_NULL_HANDLE) {
		return 0;
	}
	VkResult           result;
	VkPhysicalDevice   physical_device    = context.physical_devices.handles[context.selected_device.device_index];
	VkDevice           logical_device     = context.selected_device.handle;
	uint32_t           present_mode_count = 0;
	uint32_t           format_count       = 0;
	VkPresentModeKHR   present_modes[64];
	VkSurfaceFormatKHR formats[184];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, sc.surface, &present_mode_count, NULL);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, sc.surface, &present_mode_count, present_modes);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, sc.surface, &format_count, NULL);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, sc.surface, &format_count, formats);
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, sc.surface, &sc.surface_capabilities);
	if(result != VK_SUCCESS) {
		vkDestroySurfaceKHR(context.instance, sc.surface, NULL);
		return 0;
	}

	platform_get_window_size(window, &sc.extent.width, &sc.extent.height);
	if(sc.extent.width < sc.surface_capabilities.minImageExtent.width)
		sc.extent.width = sc.surface_capabilities.minImageExtent.width;
	else if(sc.extent.width > sc.surface_capabilities.maxImageExtent.width)
		sc.extent.width = sc.surface_capabilities.maxImageExtent.width;
	if(sc.extent.height < sc.surface_capabilities.minImageExtent.height)
		sc.extent.height = sc.surface_capabilities.minImageExtent.height;
	else if(sc.extent.height > sc.surface_capabilities.maxImageExtent.height)
		sc.extent.height = sc.surface_capabilities.maxImageExtent.height;

	sc.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	sc.color_space = formats[0].colorSpace;
	sc.format = formats[0].format;
	sc.depth_format = VK_FORMAT_D32_SFLOAT;
	for(int i = 0; i < format_count; i++) {
		VkSurfaceFormatKHR format = formats[i];
		if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			sc.format = format.format;
			sc.color_space = format.colorSpace;
			break;
		}
	}
	sc.image_count = sc.surface_capabilities.minImageCount+1;
	uint32_t max_image_count = sc.surface_capabilities.maxImageCount;
	if(max_image_count > 0 && sc.image_count > max_image_count) sc.image_count = max_image_count;

	VkSwapchainCreateInfoKHR swapchain_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	swapchain_info.surface          = sc.surface;
	swapchain_info.minImageCount    = sc.image_count;
	swapchain_info.imageFormat      = sc.format;
	swapchain_info.imageColorSpace  = sc.color_space;
	swapchain_info.imageExtent      = sc.extent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform     = sc.surface_capabilities.currentTransform;
	swapchain_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode      = sc.present_mode;
	swapchain_info.clipped          = VK_TRUE;
	result = vkCreateSwapchainKHR(logical_device, &swapchain_info, NULL, &sc.handle);
	if(result != VK_SUCCESS) {
		vkDestroySurfaceKHR(context.instance, sc.surface, NULL);
		return 0;
	}
	vkGetSwapchainImagesKHR(logical_device, sc.handle, &sc.image_count, NULL);
	sc.images       = calloc(sc.image_count, sizeof(VkImage));
	sc.image_views  = calloc(sc.image_count, sizeof(VkImageView));
	sc.depth_images = calloc(sc.image_count, sizeof(vulkan_image_t));
	vkGetSwapchainImagesKHR(logical_device, sc.handle, &sc.image_count, sc.images);

	VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format                          = sc.format;
	view_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel   = 0;
	view_info.subresourceRange.levelCount     = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount     = 1;
	for(uint32_t i = 0; i < sc.image_count; i++) {
		view_info.image = sc.images[i];
		result = vkCreateImageView(logical_device, &view_info, NULL, &sc.image_views[i]);
		if(result != VK_SUCCESS) {
			free(sc.images);
			free(sc.image_views);
			free(sc.image_views);
			vkDestroySurfaceKHR(context.instance, sc.surface, NULL);
			vkDestroySwapchainKHR(logical_device, sc.handle, NULL);
			return 0;
		}
		if(!vulkan_image_create(context, sc.depth_images+i, VK_IMAGE_TYPE_2D,
		                        sc.extent.width, sc.extent.height, sc.depth_format,
		                        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT))
		{
			vkDestroySwapchainKHR(logical_device, sc.handle, NULL);
			vkDestroySurfaceKHR(context.instance, sc.surface, NULL);
			vkDestroyImageView(logical_device, sc.image_views[i], NULL);
			return 0;
		}
		VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
		result = vkCreateSemaphore(logical_device, &semaphore_info, NULL, sc.image_avaliable_semaphores+i);
		if(result != VK_SUCCESS) {
			vkDestroySwapchainKHR(logical_device, sc.handle, NULL);
			vkDestroySurfaceKHR(context.instance, sc.surface, NULL);
			vkDestroyImageView(logical_device, sc.image_views[i], NULL);
			return 0;
		}
	}

	sc.window = window;
	*swapchain = sc;
	return 1;
}


void vulkan_swapchain_destroy(const vulkan_context_t context, vulkan_swapchain_t* swapchain) {
	vkDestroySwapchainKHR(context.selected_device.handle, swapchain->handle, NULL);
	vkDestroySurfaceKHR(context.instance, swapchain->surface, NULL);
	for(uint32_t i = 0; i < swapchain->image_count; i++) {
		vkDestroyImageView(context.selected_device.handle, swapchain->image_views[i], NULL);
		vulkan_image_destroy(context, swapchain->depth_images+i);
		vkDestroySemaphore(context.selected_device.handle, swapchain->image_avaliable_semaphores[i], NULL);
	}
	free(swapchain->image_views);
	free(swapchain->images);
	free(swapchain->depth_images);
	*swapchain = (vulkan_swapchain_t) {0};
}


int8_t vulkan_swapchain_recreate(const vulkan_context_t context, vulkan_swapchain_t* swapchain) {
	vulkan_swapchain_t sc                 = *swapchain;
	VkPhysicalDevice   physical_device    = context.physical_devices.handles[context.selected_device.device_index];
	VkDevice           logical_device     = context.selected_device.handle;
	VkResult result;

	platform_get_window_size(sc.window, &sc.extent.width, &sc.extent.height);
	if(sc.extent.width < sc.surface_capabilities.minImageExtent.width)
		sc.extent.width = sc.surface_capabilities.minImageExtent.width;
	else if(sc.extent.width > sc.surface_capabilities.maxImageExtent.width)
		sc.extent.width = sc.surface_capabilities.maxImageExtent.width;
	if(sc.extent.height < sc.surface_capabilities.minImageExtent.height)
		sc.extent.height = sc.surface_capabilities.minImageExtent.height;
	else if(sc.extent.height > sc.surface_capabilities.maxImageExtent.height)
		sc.extent.height = sc.surface_capabilities.maxImageExtent.height;
	
	VkSwapchainCreateInfoKHR swapchain_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	swapchain_info.surface          = sc.surface;
	swapchain_info.minImageCount    = sc.image_count;
	swapchain_info.imageFormat      = sc.format;
	swapchain_info.imageColorSpace  = sc.color_space;
	swapchain_info.imageExtent      = sc.extent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform     = sc.surface_capabilities.currentTransform;
	swapchain_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode      = sc.present_mode;
	swapchain_info.clipped          = VK_TRUE;
	result = vkCreateSwapchainKHR(logical_device, &swapchain_info, NULL, &sc.handle);
	if(result != VK_SUCCESS) {
		return 0;
	}
	vkGetSwapchainImagesKHR(logical_device, sc.handle, &sc.image_count, NULL);
	sc.images       = calloc(sc.image_count, sizeof(VkImage));
	sc.image_views  = calloc(sc.image_count, sizeof(VkImageView));
	sc.depth_images = calloc(sc.image_count, sizeof(vulkan_image_t));
	vkGetSwapchainImagesKHR(logical_device, sc.handle, &sc.image_count, sc.images);

	VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format                          = sc.format;
	view_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel   = 0;
	view_info.subresourceRange.levelCount     = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount     = 1;
	for(uint32_t i = 0; i < sc.image_count; i++) {
		view_info.image = sc.images[i];
		result = vkCreateImageView(logical_device, &view_info, NULL, &sc.image_views[i]);
		if(result != VK_SUCCESS) {
			free(sc.images);
			free(sc.image_views);
			free(sc.image_views);
			return 0;
		}
		if(!vulkan_image_create(context, sc.depth_images+i, VK_IMAGE_TYPE_2D,
		                        sc.extent.width, sc.extent.height, sc.depth_format,
		                        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT))
		{
			free(sc.images);
			free(sc.image_views);
			free(sc.image_views);
			return 0;
		}
	}
	vkDestroySwapchainKHR(logical_device, swapchain->handle, NULL);
	for(uint32_t i = 0; i < swapchain->image_count; i++) {
		vkDestroyImageView(logical_device, swapchain->image_views[i], NULL);
		vulkan_image_destroy(context, swapchain->depth_images+i);
	}
	*swapchain = sc;
	return 1;
}



void vulkan_swapchain_present(const vulkan_context_t context, vulkan_swapchain_t* swapchain,
                              VkSemaphore render_complete_semaphore, uint32_t image_index)
{
	VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &render_complete_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain->handle;
	present_info.pImageIndices = &image_index;
	present_info.pResults = NULL;

	VkResult r = vkQueuePresentKHR(context.selected_device.queues[GRAPHICS_QUEUE_INDEX], &present_info);
	if(r == VK_ERROR_OUT_OF_DATE_KHR) {
		vulkan_swapchain_recreate(context, swapchain);
	}
}