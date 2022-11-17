#include "vulkan_types.h"
#include "vulkan_renderer.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_pipeline.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include <malloc.h>

static const uint32_t device_extension_count = 1;
static const char* device_extension_names[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
	static const uint32_t layer_count = 0;
	static const char* layer_names[] = { NULL };
#else
	static const uint32_t layer_count = 1;
	static const char* layer_names[] = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif // NDEBUG

static vulkan_context_t context;

int8_t vulkan_renderer_init(const char* app_name) {
	context.layer_count = layer_count;
	context.layer_names = (char**)layer_names;
	context.device_extension_count = device_extension_count;
	context.device_extension_names = (char**)device_extension_names;

	VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	app_info.pApplicationName = app_name;
	app_info.pEngineName = "Vulkan Render Engine";
	app_info.apiVersion = VK_API_VERSION_1_1;

	uint32_t extension_count = 0;
	const char** extension_names = (const char**)platform_vulkan_required_extensions(&extension_count);

	VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledLayerCount = layer_count;
	instance_create_info.ppEnabledLayerNames = layer_names;
	instance_create_info.enabledExtensionCount = extension_count;
	instance_create_info.ppEnabledExtensionNames = extension_names;
	VkResult instance_result = vkCreateInstance(&instance_create_info, NULL, &context.instance);
	if(instance_result != VK_SUCCESS) return 0;

	if(vulkan_init_physical_devices(context, &context.physical_devices) == 0) {
		vkDestroyInstance(context.instance, NULL);
		return 0;
	}

	uint32_t selected_device_index = vulkan_select_physical_device(context.physical_devices);
	if(selected_device_index == -1) {
		vulkan_cleanup_physical_devices(&context.physical_devices);
		vkDestroyInstance(context.instance, NULL);
		return 0;
	}

	if(vulkan_init_device(context, &context.selected_device, selected_device_index) == 0) {
		vulkan_cleanup_physical_devices(&context.physical_devices);
		vkDestroyInstance(context.instance, NULL);
		return 0;
	}

	vec2_t renderpass_pos, renderpass_extents;
	renderpass_pos.x = 0;
	renderpass_pos.y = 0;
	renderpass_extents.x = 0;
	renderpass_extents.y = 0;
	if(!vulkan_create_renderpass(context, &context.default_renderpass, renderpass_pos, renderpass_extents, 0, 0)) {
		vulkan_cleanup_physical_devices(&context.physical_devices);
		vkDestroyInstance(context.instance, NULL);
		return 0;
	}

	return 1;
}

void vulkan_renderer_shutdown(void) {
	vulkan_destroy_renderpass(context, &context.default_renderpass);
	vulkan_cleanup_physical_devices(&context.physical_devices);
	vulkan_cleanup_device(&context.selected_device);
	vkDestroyInstance(context.instance, NULL);
}

graphics_pipeline_t* vulkan_renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info) {
	vulkan_graphics_pipeline_t* pipeline = malloc(sizeof(vulkan_graphics_pipeline_t));
	if(!vulkan_create_graphics_pipeline(context, pipeline, create_info)) return NULL;
	return (graphics_pipeline_t*)pipeline;
}

void vulkan_renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline) {
	vulkan_destroy_graphics_pipeline(context, (vulkan_graphics_pipeline_t*)pipeline);
}


int8_t vulkan_renderer_swapchain_init(swapchain_t* swapchain, platform_window_t* window) {
	vulkan_swapchain_t* sc = malloc(sizeof(vulkan_swapchain_t));
	if(!vulkan_swapchain_create(context, window, sc)) {
		return 0;
	}
	swapchain->handle = (void*)sc;
	return 1;
}
void vulkan_renderer_swapchain_cleanup(swapchain_t* swapchain) {
	vulkan_swapchain_t* sc = (vulkan_swapchain_t*)swapchain->handle;
	vulkan_swapchain_destroy(context, sc);
	swapchain->handle = NULL;
}
void vulkan_renderer_swapchain_next_framebuffer(swapchain_t swapchain, framebuffer_t* buffer) {
	vulkan_swapchain_t* sc = (vulkan_swapchain_t*)swapchain.handle;
}


int8_t vulkan_renderer_begin_draw(framebuffer_t target_buffer) {
	vulkan_command_buffer_t* cmd_buffer;
	vulkan_framebuffer_t* framebuffer;
	vulkan_command_buffer_reset(cmd_buffer);
	vulkan_command_buffer_begin_recording(cmd_buffer, 0, 0, 0);
	vulkan_renderpass_begin(&context.default_renderpass, cmd_buffer, framebuffer);

	return 1;
}

void draw_triangle(framebuffer_t target_buffer, graphics_pipeline_t* pipeline) {
	vulkan_graphics_pipeline_t* vulkan_pipeline = (vulkan_graphics_pipeline_t*)pipeline;
	vulkan_command_buffer_t* cmd_buffer;
	VkViewport viewport;
	VkRect2D scissor;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x = 0;
	scissor.offset.y = 0;

	vkCmdBindPipeline(cmd_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_pipeline->handle);

	vkCmdSetViewport(cmd_buffer->handle, 0, 1, &viewport);
	vkCmdSetScissor(cmd_buffer->handle, 0, 1, &scissor);

	vkCmdDraw(cmd_buffer->handle, 3, 1, 0, 0);
}

int8_t vulkan_renderer_end_draw(framebuffer_t target_buffer) {
	vulkan_command_buffer_t* cmd_buffer;
	vulkan_framebuffer_t* framebuffer;
	VkSemaphore image_avaliable_semaphore;
	VkSemaphore render_complete_semaphore;
	vulkan_fence_t* fence;
	vulkan_fence_reset(context, fence);

	VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	vkCreateSemaphore(context.selected_device.handle, &semaphore_info, NULL, &render_complete_semaphore);
	
	vulkan_renderpass_end(&context.default_renderpass, cmd_buffer);
	vulkan_command_buffer_end_recording(cmd_buffer);
	vulkan_command_buffer_submit(cmd_buffer);

	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer->handle;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &image_avaliable_semaphore;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &render_complete_semaphore;
	VkResult r = vkQueueSubmit(context.selected_device.queues[GRAPHICS_QUEUE_INDEX], 1, &submit_info, fence->handle);
	if(r != VK_SUCCESS) return 0;

	vulkan_fence_wait(context, fence, UINT64_MAX);
	vkDestroySemaphore(context.selected_device.handle, render_complete_semaphore, NULL);

	return 1;
}