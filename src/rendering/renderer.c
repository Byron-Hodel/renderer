#include "renderer.h"
#include "vulkan_renderer/vulkan_renderer.h"

struct backend_functions {
	void (*shutdown)(void);
	graphics_pipeline_t* (*create_graphics_pipeline)(const graphics_pipeline_create_info_t create_info);
	void (*destroy_graphics_pipeline)(graphics_pipeline_t* pipeline);
	int8_t (*swapchain_init)(swapchain_t* swapchain, platform_window_t* window);
	void (*swapchain_cleanup)(swapchain_t* swapchain);
	void (*swapchain_next_framebuffer)(swapchain_t swapchain, framebuffer_t* buffer);
	int8_t (*begin_draw)(framebuffer_t target_buffer);
	int8_t (*end_draw)(framebuffer_t target_buffer);
} functions;

int8_t renderer_init(const char* app_name, uint32_t api_backend) {
	switch (api_backend)
	{
	case RENDERER_API_BACKEND_VULKAN:
		if(!vulkan_renderer_init(app_name)) return 0;
		functions.shutdown = vulkan_renderer_shutdown;
		functions.create_graphics_pipeline = vulkan_renderer_create_graphics_pipeline;
		functions.destroy_graphics_pipeline = vulkan_renderer_destroy_graphics_pipeline;
		functions.swapchain_init = vulkan_renderer_swapchain_init;
		functions.swapchain_cleanup = vulkan_renderer_swapchain_cleanup;
		functions.swapchain_next_framebuffer = vulkan_renderer_swapchain_next_framebuffer;
		functions.begin_draw = vulkan_renderer_begin_draw;
		functions.end_draw = vulkan_renderer_end_draw;
		return 1;
	default:
		return 0;
	}
}
void renderer_shutdown(void) {
	functions.shutdown();
}

graphics_pipeline_t* renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info) {
	return functions.create_graphics_pipeline(create_info);
}

void renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline) {
	functions.destroy_graphics_pipeline(pipeline);
}

int8_t renderer_swapchain_init(swapchain_t* swapchain, platform_window_t* window) {
	return functions.swapchain_init(swapchain, window);
}
void renderer_swapchain_cleanup(swapchain_t* swapchain) {
	functions.swapchain_cleanup(swapchain);
}
void renderer_swapchain_next_framebuffer(swapchain_t swapchain, framebuffer_t* buffer) {
	functions.swapchain_next_framebuffer(swapchain, buffer);
}


int8_t renderer_begin_draw(framebuffer_t target_buffer) {
	return functions.begin_draw(target_buffer);
}
int8_t renderer_end_draw(framebuffer_t target_buffer) {
	return functions.end_draw(target_buffer);
}