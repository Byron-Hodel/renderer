#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "rendering/renderer.h"

int8_t vulkan_renderer_init(const char* app_name);
void vulkan_renderer_shutdown(void);

graphics_pipeline_t* vulkan_renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info);
void vulkan_renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline);

int8_t vulkan_renderer_init_render_target(render_target_t* render_target, void* target, uint32_t target_type);
void vulkan_renderer_cleanup_render_target(render_target_t* target);
int8_t vulkan_renderer_update_render_target(render_target_t* target);

int8_t vulkan_renderer_begin_frame(render_target_t render_target);
int8_t vulkan_renderer_submit_packet(render_target_t render_target, const render_packet_t packet);
int8_t vulkan_renderer_end_frame(render_target_t render_target);

void draw_triangle(render_target_t render_target, graphics_pipeline_t* pipeline);

#endif // VULKAN_RENDERER_H