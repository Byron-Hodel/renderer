#include "renderer.h"
#include "vulkan_renderer/vulkan_renderer.h"

struct backend_functions {
	void                 (*shutdown)(void);
	graphics_pipeline_t* (*create_graphics_pipeline)(const graphics_pipeline_create_info_t create_info);
	void                 (*destroy_graphics_pipeline)(graphics_pipeline_t* pipeline);
	int8_t               (*init_render_target)(render_target_t* render_target, void* target, uint32_t target_type);
	void                 (*cleanup_render_target)(render_target_t* target);
	int8_t               (*update_render_target)(render_target_t* target);
	int8_t               (*begin_frame)(render_target_t render_target);
	int8_t               (*submit_packet)(render_target_t render_target, const render_packet_t packet);
	int8_t               (*end_frame)(render_target_t render_target);
} functions;

int8_t renderer_init(const char* app_name, uint32_t api_backend) {
	switch (api_backend)
	{
	case RENDERER_API_BACKEND_VULKAN:
		if(!vulkan_renderer_init(app_name)) return 0;
		functions.shutdown = vulkan_renderer_shutdown;
		functions.create_graphics_pipeline = vulkan_renderer_create_graphics_pipeline;
		functions.destroy_graphics_pipeline = vulkan_renderer_destroy_graphics_pipeline;
		functions.init_render_target = vulkan_renderer_init_render_target;
		functions.cleanup_render_target = vulkan_renderer_cleanup_render_target;
		functions.update_render_target = vulkan_renderer_update_render_target;
		functions.begin_frame = vulkan_renderer_begin_frame;
		functions.submit_packet = vulkan_renderer_submit_packet;
		functions.end_frame = vulkan_renderer_end_frame;
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

int8_t renderer_init_render_target(render_target_t* render_target, void* target, uint32_t target_type) {
	return functions.init_render_target(render_target, target, target_type);
}
void renderer_cleanup_render_target(render_target_t* target) {
	functions.cleanup_render_target(target);
}
int8_t renderer_update_render_target(render_target_t* target) {
	return functions.update_render_target(target);
}

int8_t renderer_begin_frame(render_target_t render_target) {
	return functions.begin_frame(render_target);
}
int8_t renderer_submit_packet(render_target_t render_target, const render_packet_t packet) {
	return functions.submit_packet(render_target, packet);
}
int8_t renderer_end_frame(render_target_t render_target) {
	return functions.end_frame(render_target);
}