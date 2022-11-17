#ifndef RENDERER_H
#define RENDERER_H

#include <platform/platform.h>
#include <stdint.h>

#define RENDERER_API_BACKEND_VULKAN  1
//#define RENDERER_API_BACKEND_DIRECTX 2

typedef struct swapchain_t {
	void* handle;
} swapchain_t;

typedef struct framebuffer_t {
	void* handle;
	void* extra;
} framebuffer_t;

typedef struct graphics_pipeline_t graphics_pipeline_t;

typedef struct {
	char*    vert_shader_src;
	uint64_t vert_shader_src_len;
	char*    frag_shader_src;
	uint64_t frag_shader_src_len;
} graphics_pipeline_create_info_t;

int8_t renderer_init(const char* app_name, uint32_t api_backend);
void renderer_shutdown(void);

graphics_pipeline_t* renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info);
void renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline);

int8_t renderer_swapchain_init(swapchain_t* swapchain, platform_window_t* window);
void renderer_swapchain_cleanup(swapchain_t* swapchain);
void renderer_swapchain_next_framebuffer(swapchain_t swapchain, framebuffer_t* buffer);

int8_t renderer_begin_draw(framebuffer_t target_buffer);
int8_t renderer_end_draw(framebuffer_t target_buffer);


#endif // RENDERER_H