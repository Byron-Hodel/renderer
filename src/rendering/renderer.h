#ifndef RENDERER_H
#define RENDERER_H

#include <platform/platform.h>
#include <stdint.h>

#define RENDERER_API_BACKEND_VULKAN  1
//#define RENDERER_API_BACKEND_DIRECTX 2

#define RENDER_TARGET_WINDOW      1
#define RENDER_TARGET_FRAMEBUFFER 2

typedef struct renderer_graphics_pipeline_t {
	void* handle;
} renderer_graphics_pipeline_t;

typedef struct graphics_pipeline_t graphics_pipeline_t;

typedef struct {
	char*    vert_shader_src;
	uint64_t vert_shader_src_len;
	char*    frag_shader_src;
	uint64_t frag_shader_src_len;
} graphics_pipeline_create_info_t;

typedef struct mesh_render_packet_t {
	// mesh render packet stuff
} mesh_render_packet_t;

typedef struct render_packet_t {
	uint32_t packet_type;
	union {
		mesh_render_packet_t mesh_packet;
	};
} render_packet_t;

typedef struct render_target_t {
	uint32_t type;
	void*    target_handle;
	void*    handle;
} render_target_t;


int8_t renderer_init(const char* app_name, uint32_t api_backend);
void renderer_shutdown(void);

graphics_pipeline_t* renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info);
void renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline);

int8_t renderer_init_render_target(render_target_t* render_target, void* target, uint32_t target_type);
void renderer_cleanup_render_target(render_target_t* target);
int8_t renderer_update_render_target(render_target_t* target);

int8_t renderer_begin_frame(render_target_t render_target);
int8_t renderer_submit_packet(render_target_t render_target, const render_packet_t packet);
int8_t renderer_end_frame(render_target_t render_target);

#endif // RENDERER_H