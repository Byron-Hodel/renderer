#include <platform/platform.h>
#include <stdlib.h>
#include "rendering/vulkan_renderer/vulkan_renderer.h"

#include "rendering/renderer.h"
#include "resource_system.h"

#include <stdio.h>
#include <string.h>

#include <glslang/Include/glslang_c_interface.h>

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
	window_create_info.x = 100;
	window_create_info.y = 100;
	window_create_info.width = 1920>>1;
	window_create_info.height = 1080>>1;
	window_create_info.parent = NULL;
	window_create_info.flags = PLATFORM_WF_SPLASH;
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

	swapchain_t window_swapchain;
	if(!renderer_swapchain_init(&window_swapchain, window)) {
		platform_destroy_window(window, NULL);
		platform_shutdown();
		renderer_shutdown();
		return -1;
	}

	while(!platform_window_should_close(window)) {
		platform_handle_events();
		framebuffer_t framebuffer;
		renderer_swapchain_next_framebuffer(window_swapchain, &framebuffer);
		if(renderer_begin_draw(framebuffer)) {
			draw_triangle(framebuffer, pipeline);
			renderer_end_draw(framebuffer);
		}
		platform_sleep_miliseconds(32);
	}


	renderer_destroy_graphics_pipeline(pipeline);
	renderer_swapchain_cleanup(&window_swapchain);

	platform_destroy_window(window, NULL);

	platform_shutdown();
	renderer_shutdown(); // shutdown renderer only after platform has shutdown

	resource_system_shutdown();
	return 0;
}





static glslang_resource_t default_resource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,
	/* .maxDualSourceDrawBuffersEXT = */ 1,

	/* .limits = */ (glslang_limits_t) {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1
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