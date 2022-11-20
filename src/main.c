#include <platform/platform.h>
#include <stdlib.h>
#include "rendering/vulkan_renderer/vulkan_renderer.h"

#include "rendering/renderer.h"
#include "resource_system.h"
#include "math.h"

#include <stdio.h>
#include <string.h>

#include <glslang/Include/glslang_c_interface.h>

typedef struct vertex_t {
	vec2_t pos;
	vec3_t color;
} vertex_t;

uint32_t vertex_count = 3;
vertex_t test_verticies[] = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

static uint64_t file_size(const char* file_path) {
	FILE* file = fopen(file_path, "r");
	if(file == NULL) return 0;

	fseek(file, 0, SEEK_END);
	uint32_t file_size = ftell(file);
	fclose(file);
	return file_size;
}

void read_file(char* file_path, uint64_t file_size, char* buffer) {
	FILE* file = fopen(file_path, "r");
	//if(file == NULL) return -1;

	uint64_t bytes_read = 0;
	while(bytes_read != file_size) {
		bytes_read += fread(buffer, 1, file_size - bytes_read, file);
	}
	fclose(file);
}

static uint32_t* compile_shader(char* shader_text, glslang_stage_t stage, uint64_t* size);

int main(void) {
	// not using resource system yet
	//resource_system_init();
	//uint64_t vert_shader_size = file_size("assets/shaders/shader.vert");
	//uint64_t frag_shader_size = file_size("assets/shaders/shader.frag");

	uint64_t triangle_shader_size = file_size("assets/shaders/triangle.glsl");
	char* triangle_shader_text = (char*)malloc(triangle_shader_size+1);

	char* triangle_vert_text;
	char* triangle_frag_text;
	uint64_t traingle_vert_text_len;
	uint64_t traingle_frag_text_len;

	FILE* shader_file = fopen("assets/shaders/triangle.glsl", "r");
	if(shader_file == NULL) return -1;

	uint64_t bytes_read = 0;
	while(bytes_read != triangle_shader_size) {
		bytes_read += fread(triangle_shader_text, 1, triangle_shader_size - bytes_read, shader_file);
	}
	fclose(shader_file);

	triangle_vert_text = strstr(triangle_shader_text, "#type vertex");
	triangle_frag_text = strstr(triangle_shader_text, "#type fragment");
	triangle_vert_text += 12;
	// assumes frag shader is right after vert shader
	// fix later
	traingle_vert_text_len = (uint64_t)(triangle_frag_text - triangle_vert_text);
	triangle_frag_text += 14;
	// assumes no shader comes after frag shader
	// fix later
	traingle_frag_text_len = triangle_shader_size - (uint64_t)(triangle_frag_text - triangle_shader_text);

	triangle_vert_text[traingle_vert_text_len] = '\0';
	triangle_frag_text[traingle_frag_text_len] = '\0';

	uint64_t vert_spirv_size;
	uint64_t frag_spirv_size;
	uint32_t* vert_spirv = compile_shader(triangle_vert_text, GLSLANG_STAGE_VERTEX, &vert_spirv_size);
	uint32_t* frag_spirv = compile_shader(triangle_frag_text, GLSLANG_STAGE_FRAGMENT, &frag_spirv_size);
	if(vert_spirv == NULL || frag_spirv == NULL) return -1;


	if(!platform_init(NULL)) return -1;

	if(!renderer_init("Renderer", RENDERER_API_BACKEND_VULKAN)) {
		platform_shutdown();
		return -1;
	}


	platform_window_create_info_t window_create_info = {0};
	window_create_info.name = (char*)"Cat Window";
	window_create_info.x = 0;
	window_create_info.y = 0;
	window_create_info.width = 1920;
	window_create_info.height = 1080;
	window_create_info.parent = NULL;
	window_create_info.flags = PLATFORM_WF_NORMAL;
	platform_window_t* window = platform_create_window(window_create_info, NULL);

	graphics_pipeline_create_info_t pipeline_info;
	pipeline_info.vert_shader_src = (char*)vert_spirv;
	pipeline_info.vert_shader_src_len = vert_spirv_size;
	pipeline_info.frag_shader_src = (char*)frag_spirv;
	pipeline_info.frag_shader_src_len = frag_spirv_size;
	graphics_pipeline_t* pipeline = renderer_create_graphics_pipeline(pipeline_info);
	if(pipeline == NULL) {
		platform_destroy_window(window, NULL);
		platform_shutdown();
		renderer_shutdown();
		return -1;
	}

	uint32_t last_width, last_height;
	platform_get_window_size(window, &last_width, &last_height);

	render_target_t render_target;
	renderer_init_render_target(&render_target, window, RENDER_TARGET_WINDOW);



	uint32_t frame = 0;
	while(!platform_window_should_close(window)) {
		platform_handle_events();
		uint32_t width, height;
		platform_get_window_size(window, &width, &height);
		if(width != last_width || height != last_height) {
			renderer_update_render_target(&render_target);
			last_width = width;
			last_height = height;
		}
		renderer_begin_frame(render_target);
		//render_packet_t packet = {0};
		//renderer_submit_packet(&frame_state, packet);
		draw_triangle(render_target, pipeline);
		renderer_end_frame(render_target);
		//platform_sleep_miliseconds(32);
		printf("frame: %d\r", frame++);
		fflush(stdout);
	}


	renderer_cleanup_render_target(&render_target);

	renderer_destroy_graphics_pipeline(pipeline);

	platform_destroy_window(window, NULL);

	platform_shutdown();
	renderer_shutdown(); // shutdown renderer only after platform has shutdown

	resource_system_shutdown();
	return 0;
}





static glslang_resource_t default_resource = {
	.max_lights = 32,
	.max_clip_planes = 6,
	.max_texture_units = 32,
	.max_texture_coords = 32,
	.max_vertex_attribs = 64,
	.max_vertex_uniform_components = 4096,
	.max_varying_floats = 64,
	.max_vertex_texture_image_units = 32,
	.max_combined_texture_image_units = 80,
	.max_texture_image_units = 32,
	.max_fragment_uniform_components = 4096,
	.max_draw_buffers = 32,
	.max_vertex_uniform_vectors = 128,
	.max_varying_vectors = 8,
	.max_fragment_uniform_vectors = 16,
	.max_vertex_output_vectors = 16,
	.max_fragment_input_vectors = 15,
	.min_program_texel_offset = -8,
	.max_program_texel_offset = 7,
	.max_clip_distances = 8,
	.max_compute_work_group_count_x = 65535,
	.max_compute_work_group_count_y = 65535,
	.max_compute_work_group_count_z = 65535,
	.max_compute_work_group_size_x = 1024,
	.max_compute_work_group_size_y = 1024,
	.max_compute_work_group_size_z = 64,
	.max_compute_uniform_components = 1024,
	.max_compute_texture_image_units = 16,
	.max_compute_image_uniforms = 8,
	.max_compute_atomic_counters = 8,
	.max_compute_atomic_counter_buffers = 1,
	.max_varying_components = 60,
	.max_vertex_output_components = 64,
	.max_geometry_input_components = 64,
	.max_geometry_output_components = 128,
	.max_fragment_input_components = 128,
	.max_image_units = 8,
	.max_combined_image_units_and_fragment_outputs = 8,
	.max_combined_shader_output_resources = 8,
	.max_image_samples = 0,
	.max_vertex_image_uniforms = 0,
	.max_tess_control_image_uniforms = 0,
	.max_tess_evaluation_image_uniforms = 0,
	.max_geometry_image_uniforms = 0,
	.max_fragment_image_uniforms = 8,
	.max_combined_image_uniforms = 8,
	.max_geometry_texture_image_units = 16,
	.max_geometry_output_vertices = 256,
	.max_geometry_total_output_components = 1024,
	.max_geometry_uniform_components = 1024,
	.max_geometry_varying_components = 64,
	.max_tess_control_input_components = 128,
	.max_tess_control_output_components = 128,
	.max_tess_control_texture_image_units = 16,
	.max_tess_control_uniform_components = 1024,
	.max_tess_control_total_output_components = 4096,
	.max_tess_evaluation_input_components = 128,
	.max_tess_evaluation_output_components = 128,
	.max_tess_evaluation_texture_image_units = 16,
	.max_tess_evaluation_uniform_components = 1024,
	.max_tess_patch_components = 120,
	.max_patch_vertices = 32,
	.max_tess_gen_level = 64,
	.max_viewports = 16,
	.max_vertex_atomic_counters = 0,
	.max_tess_control_atomic_counters = 0,
	.max_tess_evaluation_atomic_counters = 0,
	.max_geometry_atomic_counters = 0,
	.max_fragment_atomic_counters = 8,
	.max_combined_atomic_counters = 8,
	.max_atomic_counter_bindings = 1,
	.max_vertex_atomic_counter_buffers = 0,
	.max_tess_control_atomic_counter_buffers = 0,
	.max_tess_evaluation_atomic_counter_buffers = 0,
	.max_geometry_atomic_counter_buffers = 0,
	.max_fragment_atomic_counter_buffers = 1,
	.max_combined_atomic_counter_buffers = 1,
	.max_atomic_counter_buffer_size = 16384,
	.max_transform_feedback_buffers = 4,
	.max_transform_feedback_interleaved_components = 64,
	.max_cull_distances = 8,
	.max_combined_clip_and_cull_distances = 8,
	.max_samples = 4,
	.max_mesh_output_vertices_nv = 256,
	.max_mesh_output_primitives_nv = 512,
	.max_mesh_work_group_size_x_nv = 32,
	.max_mesh_work_group_size_y_nv = 1,
	.max_mesh_work_group_size_z_nv = 1,
	.max_task_work_group_size_x_nv = 32,
	.max_task_work_group_size_y_nv = 1,
	.max_task_work_group_size_z_nv = 1,
	.max_mesh_view_count_nv = 4,
	.maxDualSourceDrawBuffersEXT = 1,

	.limits = (glslang_limits_t) {
		.non_inductive_for_loops = 1,
		.while_loops = 1,
		.do_while_loops = 1,
		.general_uniform_indexing = 1,
		.general_attribute_matrix_vector_indexing = 1,
		.general_varying_indexing = 1,
		.general_sampler_indexing = 1,
		.general_variable_indexing = 1,
		.general_constant_matrix_vector_indexing = 1
	}
};

static uint32_t* compile_shader(char* shader_text, glslang_stage_t stage, uint64_t* size) {
	glslang_initialize_process();

	glslang_input_t input = {0};
	input.language = GLSLANG_SOURCE_GLSL;
	input.stage = stage;
	input.client = GLSLANG_CLIENT_VULKAN;
	input.client_version = GLSLANG_TARGET_VULKAN_1_1;
	input.target_language = GLSLANG_TARGET_SPV;
	input.target_language_version = GLSLANG_TARGET_SPV_1_3;
	input.code = shader_text;
	input.default_version = 100;
	input.default_profile = GLSLANG_NO_PROFILE;
	input.force_default_version_and_profile = false;
	input.forward_compatible = false;
	input.messages = GLSLANG_MSG_DEFAULT_BIT;
	input.resource = &default_resource;

	glslang_shader_t* shader = glslang_shader_create(&input);
	if(shader == NULL) return NULL;

	if(!glslang_shader_preprocess(shader, &input)) {
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		glslang_shader_delete(shader);
		glslang_finalize_process();
		platform_terminal_print_error("Shader Preprocess Step Failed.\n", PLATFORM_COLOR_BRIGHT_RED, 0, PLATFORM_TEXT_BOLD);
		return NULL;
	}

	if(!glslang_shader_parse(shader, &input)) {
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		glslang_shader_delete(shader);
		glslang_finalize_process();
		platform_terminal_print_error("Shader Parse Step Failed.\n", PLATFORM_COLOR_BRIGHT_RED, 0, PLATFORM_TEXT_BOLD);
		return NULL;
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if(!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		printf("%s\n", glslang_shader_get_info_log(shader));
		printf("%s\n", glslang_shader_get_info_debug_log(shader));
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		glslang_finalize_process();
		printf("Shader Program Link Step Failed.\n");
		return NULL;
	}

	glslang_program_SPIRV_generate(program, stage);

	uint64_t spirv_len = glslang_program_SPIRV_get_size(program);
	*size = spirv_len * sizeof(uint32_t);
	uint32_t* spirv = calloc(sizeof(uint32_t), spirv_len);
	glslang_program_SPIRV_get(program, spirv);

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	glslang_finalize_process();
	return spirv;
}