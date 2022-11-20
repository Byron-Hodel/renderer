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
	if(!vulkan_create_renderpass(&context, &context.default_renderpass, renderpass_pos, renderpass_extents, 0, 0)) {
		vulkan_cleanup_physical_devices(&context.physical_devices);
		vkDestroyInstance(context.instance, NULL);
		return 0;
	}

	return 1;
}




void vulkan_renderer_shutdown(void) {
	vulkan_destroy_renderpass(&context, &context.default_renderpass);
	vulkan_cleanup_physical_devices(&context.physical_devices);
	vulkan_cleanup_device(&context.selected_device);
	vkDestroyInstance(context.instance, NULL);
}




graphics_pipeline_t* vulkan_renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info) {
	vulkan_graphics_pipeline_t* pipeline = malloc(sizeof(vulkan_graphics_pipeline_t));
	if(!vulkan_create_graphics_pipeline(&context, pipeline, create_info)) return NULL;
	return (graphics_pipeline_t*)pipeline;
}




void vulkan_renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline) {
	vulkan_destroy_graphics_pipeline(&context, (vulkan_graphics_pipeline_t*)pipeline);
}




int8_t vulkan_renderer_init_render_target(render_target_t* render_target, void* target, uint32_t target_type) {
	if(target_type == RENDER_TARGET_WINDOW) {
		render_target->type = RENDER_TARGET_WINDOW;
		render_target->target_handle = target;
		platform_window_t* window = (platform_window_t*)target;
		vulkan_window_render_target_t* handle = malloc(sizeof(vulkan_window_render_target_t));
		if(!vulkan_command_buffer_alloc(context, &handle->command_buffer, context.selected_device.graphics_cmd_pool, 1)) {
			return 0;
		}
		handle->in_flight_fence.signaled = 1; // start the fence in the signaled state
		if(!vulkan_fence_create(context, &handle->in_flight_fence)) {
			vulkan_command_buffer_free(context, &handle->command_buffer, context.selected_device.graphics_cmd_pool);
			free(handle);
			return 0;
		}
		if(!vulkan_swapchain_create(&context, window, &handle->swapchain)) {
			vulkan_command_buffer_free(context, &handle->command_buffer, context.selected_device.graphics_cmd_pool);
			vulkan_fence_destroy(context, &handle->in_flight_fence);
			free(handle);
			return 0;
		}
		VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
		vkCreateSemaphore(context.selected_device.handle, &semaphore_info, NULL, &handle->image_available_semaphore);
		vkCreateSemaphore(context.selected_device.handle, &semaphore_info, NULL, &handle->render_complete_semaphore);
		render_target->handle = handle;
		return 1;
	}
	return 0;
}




void vulkan_renderer_cleanup_render_target(render_target_t* target) {
	vkDeviceWaitIdle(context.selected_device.handle);
	if(target->type == RENDER_TARGET_WINDOW) {
		target->type = 0;
		target->target_handle = NULL;
		vulkan_window_render_target_t* handle = (vulkan_window_render_target_t*)target->handle;
		target->handle = NULL;
		vulkan_command_buffer_free(context, &handle->command_buffer, context.selected_device.graphics_cmd_pool);
		vulkan_fence_destroy(context, &handle->in_flight_fence);
		vulkan_swapchain_destroy(&context, &handle->swapchain);
		vkDestroySemaphore(context.selected_device.handle, handle->image_available_semaphore, NULL);
		vkDestroySemaphore(context.selected_device.handle, handle->render_complete_semaphore, NULL);
		free(handle);
	}
}




int8_t vulkan_renderer_update_render_target(render_target_t* target) {
	if(target->type == RENDER_TARGET_WINDOW) {
		vulkan_window_render_target_t* handle = (vulkan_window_render_target_t*)target->handle;
		return vulkan_swapchain_recreate(&context, &handle->swapchain);
	}
	return 0;
}




int8_t vulkan_renderer_begin_frame(render_target_t render_target) {
	vulkan_framebuffer_t* framebuffer;
	vulkan_command_buffer_t* command_buffer;
	vulkan_fence_t* in_flight_fence;
	if(render_target.type == RENDER_TARGET_WINDOW) {
		vulkan_window_render_target_t* handle = (vulkan_window_render_target_t*)render_target.handle;
		command_buffer = &handle->command_buffer;
		vulkan_fence_wait(context, &handle->in_flight_fence, UINT64_MAX);
		vulkan_fence_reset(context, &handle->in_flight_fence);
		VkResult result = vkAcquireNextImageKHR(context.selected_device.handle, handle->swapchain.handle, UINT64_MAX,
	                                            handle->image_available_semaphore, VK_NULL_HANDLE, &handle->image_index);
		if(result != VK_SUCCESS) return 0;
		in_flight_fence = &handle->in_flight_fence;
	}
	vulkan_command_buffer_reset(command_buffer);
	vulkan_command_buffer_begin_recording(command_buffer, 0, 0, 0);
	return 1;
}



int8_t vulkan_renderer_submit_packet(render_target_t render_target, const render_packet_t packet) {
	return 1;
}



int8_t vulkan_renderer_end_frame(render_target_t render_target) {
	vulkan_command_buffer_t* command_buffer;
	VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	vulkan_fence_t in_flight_fence = {0};
	if(render_target.type == RENDER_TARGET_WINDOW) {
		vulkan_window_render_target_t* handle = (vulkan_window_render_target_t*)render_target.handle;
		command_buffer = &handle->command_buffer;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &handle->image_available_semaphore;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &handle->render_complete_semaphore;
		in_flight_fence = handle->in_flight_fence;
	}
	vulkan_command_buffer_end_recording(command_buffer);
	vulkan_command_buffer_submit(command_buffer);
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer->handle;
	vkQueueSubmit(context.selected_device.queues[GRAPHICS_QUEUE_INDEX], 1, &submit_info, in_flight_fence.handle);

	if(render_target.type == RENDER_TARGET_WINDOW) {
		vulkan_window_render_target_t* handle = (vulkan_window_render_target_t*)render_target.handle;
		VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &handle->render_complete_semaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &handle->swapchain.handle;
		present_info.pImageIndices = &handle->image_index;
		vkQueuePresentKHR(context.selected_device.queues[GRAPHICS_QUEUE_INDEX], &present_info);
	}
	return 1;
}


void draw_triangle(render_target_t render_target, graphics_pipeline_t* pipeline) {
	vulkan_window_render_target_t* handle = (vulkan_window_render_target_t*)render_target.handle;

	vulkan_graphics_pipeline_t* vulkan_pipeline = (vulkan_graphics_pipeline_t*)pipeline;
	vulkan_command_buffer_t* command_buffer = &handle->command_buffer;

	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)handle->swapchain.extent.width;
	viewport.height = (float)handle->swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = handle->swapchain.extent;

	VkClearValue clear_colors[2];
	clear_colors[0] = (VkClearValue) {{{ 64.34f/70, 38.587f/70, 98.661f/70, 1.0f}}};
	clear_colors[1] = (VkClearValue) {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	begin_info.renderPass = context.default_renderpass.handle;
	begin_info.framebuffer = handle->swapchain.default_renderpass_frame_buffers[handle->image_index].handle;
	begin_info.renderArea.offset.x = (int32_t)context.default_renderpass.pos.x;
	begin_info.renderArea.offset.y = (int32_t)context.default_renderpass.pos.y;
	begin_info.renderArea.extent = handle->swapchain.extent;
	begin_info.clearValueCount = 2;
	begin_info.pClearValues = clear_colors;

	//vulkan_command_buffer_reset(command_buffer);
	//vulkan_command_buffer_begin_recording(command_buffer, 0, 0, 0);
	vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_pipeline->handle);
	vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);
	vkCmdDraw(command_buffer->handle, 3, 1, 0, 0);
	vkCmdEndRenderPass(command_buffer->handle);
	//vulkan_command_buffer_end_recording(command_buffer);

	//VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	//VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	//submit_info.waitSemaphoreCount = 1;
	//submit_info.pWaitSemaphores = &handle->image_available_semaphore;
	//submit_info.pWaitDstStageMask = wait_stages;
	//submit_info.commandBufferCount = 1;
	//submit_info.pCommandBuffers = &command_buffer->handle;
	//submit_info.signalSemaphoreCount = 1;
	//submit_info.pSignalSemaphores = &handle->render_complete_semaphore;
	//vkQueueSubmit(context.selected_device.queues[GRAPHICS_QUEUE_INDEX], 1, &submit_info, handle->in_flight_fence.handle);

	//VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	//present_info.waitSemaphoreCount = 1;
	//present_info.pWaitSemaphores = &handle->render_complete_semaphore;
	//present_info.swapchainCount = 1;
	//present_info.pSwapchains = &handle->swapchain.handle;
	//present_info.pImageIndices = &image_index;
	//vkQueuePresentKHR(context.selected_device.queues[GRAPHICS_QUEUE_INDEX], &present_info);
}