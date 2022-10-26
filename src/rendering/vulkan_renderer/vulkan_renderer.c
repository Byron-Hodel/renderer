#define PLATFORM_VULKAN
#include "vulkan_renderer.h"
#include "vulkan_internal.h"
#include <malloc.h>

#ifdef NDEBUG
	const uint32_t layer_count = 0;
	const char* layer_names[] = { NULL };
#else
	const uint32_t layer_count = 1;
	const char* layer_names[] = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif // NDEBUG

const uint32_t device_extension_count = 1;
const char* device_extension_names[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


int8_t vulkan_init_context(vulkan_context_t* context, platform_context_t* platform_context, const char* app_name) {
	VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	app_info.pApplicationName = app_name;
	app_info.pEngineName = "Vulkan Render Engine";
	app_info.apiVersion = VK_API_VERSION_1_1;

	uint32_t extension_count = 0;
	const char** extension_names = (const char**)platform_vulkan_required_extensions(platform_context, &extension_count);

	VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledLayerCount = layer_count;
	instance_create_info.ppEnabledLayerNames = layer_names;
	instance_create_info.enabledExtensionCount = extension_count;
	instance_create_info.ppEnabledExtensionNames = extension_names;
	VkResult instance_result = vkCreateInstance(&instance_create_info, NULL, &context->instance);
	if(instance_result != VK_SUCCESS) return 0;

	if(!_vulkan_init_physical_devices(context)) {
		vkDestroyInstance(context->instance, NULL);
		return 0;
	}
	if(!_vulkan_select_physical_device(context)) {
		vkDestroyInstance(context->instance, NULL);
		return 0;
	}
	if(!_vulkan_init_selected_device(context)) {
		vkDestroyInstance(context->instance, NULL);
		return 0;
	}

	return 1;
}
void vulkan_cleanup_context(vulkan_context_t* context) {
	_vulkan_cleanup_physical_device(context);
	_vulkan_cleanup_selected_device(context);
	vkDestroyInstance(context->instance, NULL);
	context->instance = VK_NULL_HANDLE;
}

render_target_t* vulkan_create_render_target(renderer_context_t* context, platform_window_t* platform_window) {
	render_target_t* render_target = malloc(sizeof(render_target_t));
	render_target->window = platform_window;
	render_target->surface = platform_vulkan_create_surface(context->platform_context, platform_window, context->vulkan.instance);
	if(render_target->surface == VK_NULL_HANDLE) {
		free(render_target);
		return NULL;
	}
	_vulkan_get_swapchain_modes(&context->vulkan, render_target);
	if(!_vulkan_create_swapchain(context->platform_context, &context->vulkan, render_target)) {
		free(render_target);
		vkDestroySurfaceKHR(context->vulkan.instance, render_target->surface, NULL);
	}
	return render_target;
}

void vulkan_destroy_render_target(renderer_context_t* context, render_target_t* target) {
	_vulkan_destroy_swapchain(&context->vulkan, target);
	vkDestroySurfaceKHR(context->vulkan.instance, target->surface, NULL);
	free(target);
}

int8_t vulkan_recreate_render_target(renderer_context_t* context, render_target_t* target) {
	_vulkan_destroy_swapchain(&context->vulkan, target);
	_vulkan_create_swapchain(context->platform_context, &context->vulkan, target);
	return 1;
}

void vulkan_draw_triangle(renderer_context_t* context, render_target_t* target) {

	VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphore image_avaliable_semaphore;
	VkSemaphore render_finished_semaphore;
	VkFence in_flight_fence;

	VkResult r;

	r = vkCreateSemaphore(context->vulkan.selected_device.logical_device,
	                      &semaphore_info, NULL, &image_avaliable_semaphore);
	if(r != VK_SUCCESS) return;
	r = vkCreateSemaphore(context->vulkan.selected_device.logical_device,
	                  &semaphore_info, NULL, &render_finished_semaphore);
	if(r != VK_SUCCESS) return;
	r = vkCreateFence(context->vulkan.selected_device.logical_device,
	                  &fence_info, NULL, &in_flight_fence);
	if(r != VK_SUCCESS) return;

	vkWaitForFences(context->vulkan.selected_device.logical_device,
	                1, &in_flight_fence, VK_TRUE, UINT64_MAX);
	
	vkResetFences(context->vulkan.selected_device.logical_device,
	             1, &in_flight_fence);

	uint32_t image_index = 0;
	vkAcquireNextImageKHR(context->vulkan.selected_device.logical_device,
	                      target->swapchain, UINT64_MAX, image_avaliable_semaphore,
	                      VK_NULL_HANDLE, &image_index);



	FILE* vert_file = fopen("vert.spv", "r");
	if(vert_file == NULL) {
		return;
	}
	fseek(vert_file, 0, SEEK_END);
	uint32_t vert_file_size = ftell(vert_file);
	fseek(vert_file, 0, SEEK_SET);
	char* vert_code = calloc(sizeof(char), vert_file_size);
	uint32_t bytes_read = 0;
	while(bytes_read != vert_file_size) {
		bytes_read += fread(vert_code, sizeof(char), vert_file_size - bytes_read, vert_file);
	}
	fclose(vert_file);
	VkShaderModule vert_shader_module;
	VkShaderModuleCreateInfo vert_module_create_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	vert_module_create_info.pCode = (uint32_t*)vert_code;
	vert_module_create_info.codeSize = vert_file_size;

	VkResult vert_result = vkCreateShaderModule(context->vulkan.selected_device.logical_device,
	                                            &vert_module_create_info, NULL, &vert_shader_module);

	FILE* frag_file = fopen("frag.spv", "r");
	if(frag_file == NULL) {
		
	}
	fseek(frag_file, 0, SEEK_END);
	uint32_t frag_file_size = ftell(frag_file);
	fseek(frag_file, 0, SEEK_SET);
	char* frag_code = calloc(sizeof(char), frag_file_size);
	bytes_read = 0;
	while(bytes_read != frag_file_size) {
		bytes_read += fread(frag_code, sizeof(char), frag_file_size - bytes_read, frag_file);
	}
	fclose(frag_file);
	VkShaderModule frag_shader_module;
	VkShaderModuleCreateInfo frag_module_create_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	frag_module_create_info.pCode = (uint32_t*)frag_code;
	frag_module_create_info.codeSize = frag_file_size;

	VkResult frag_result = vkCreateShaderModule(context->vulkan.selected_device.logical_device,
	                                            &frag_module_create_info, NULL, &frag_shader_module);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	vert_shader_stage_info.pName = "main";
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	frag_shader_stage_info.pName = "main";
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineShaderStageCreateInfo shader_stages[2] = {vert_shader_stage_info, frag_shader_stage_info};


	VkPipelineVertexInputStateCreateInfo vert_state_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vert_state_info.vertexBindingDescriptionCount = 0;
	vert_state_info.pVertexBindingDescriptions = NULL;
	vert_state_info.vertexAttributeDescriptionCount = 0;
	vert_state_info.pVertexAttributeDescriptions = NULL;


	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = target->swapchain_extent.width;
	viewport.height = target->swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor_rect;
	scissor_rect.offset.x = 0;
	scissor_rect.offset.y = 0;
	scissor_rect.extent = target->swapchain_extent;

	VkDynamicState dynamic_states[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	dynamic_state_info.dynamicStateCount = 2;
	dynamic_state_info.pDynamicStates = dynamic_states;

	VkPipelineViewportStateCreateInfo viewport_state_info = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = &viewport;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = &scissor_rect;

	VkPipelineRasterizationStateCreateInfo rasterizer_info = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer_info.depthBiasEnable = VK_FALSE;
	rasterizer_info.depthBiasConstantFactor = 0.0f;
	rasterizer_info.depthBiasClamp = 0.0f;
	rasterizer_info.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisample_info = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisample_info.sampleShadingEnable = VK_FALSE;
	multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_info.minSampleShading = 1.0f;
	multisample_info.pSampleMask = NULL;
	multisample_info.alphaToCoverageEnable = VK_FALSE;
	multisample_info.alphaToCoverageEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_state;
	color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_state.blendEnable = VK_FALSE;
	color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_state;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipeline_layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = NULL;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = NULL;

	VkPipelineLayout pipeline_layout;
	r =vkCreatePipelineLayout(context->vulkan.selected_device.logical_device,
	                          &pipeline_layout_info, NULL, &pipeline_layout);
	if(r != VK_SUCCESS) return;



	VkAttachmentDescription color_attachment = {0};
	color_attachment.format = target->swapchain_format.format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {0};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPass render_pass;
	VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	r =vkCreateRenderPass(context->vulkan.selected_device.logical_device,
	                      &render_pass_info, NULL, &render_pass);
	if(r != VK_SUCCESS) return;

	VkGraphicsPipelineCreateInfo graphics_pipeline_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	graphics_pipeline_info.stageCount = 2;
	graphics_pipeline_info.pStages = shader_stages;
	graphics_pipeline_info.pVertexInputState = &vert_state_info;
	graphics_pipeline_info.pInputAssemblyState = &input_assembly_info;
	graphics_pipeline_info.pViewportState = &viewport_state_info;
	graphics_pipeline_info.pRasterizationState = &rasterizer_info;
	graphics_pipeline_info.pMultisampleState = &multisample_info;
	graphics_pipeline_info.pDepthStencilState = NULL;
	graphics_pipeline_info.pColorBlendState = &color_blending;
	graphics_pipeline_info.pDynamicState = &dynamic_state_info;
	graphics_pipeline_info.layout = pipeline_layout;
	graphics_pipeline_info.renderPass = render_pass;
	graphics_pipeline_info.subpass = 0;
	graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	graphics_pipeline_info.basePipelineIndex = -1;

	VkPipeline graphics_pipeline = VK_NULL_HANDLE;
	r = vkCreateGraphicsPipelines(context->vulkan.selected_device.logical_device,
	                             VK_NULL_HANDLE, 1, &graphics_pipeline_info, NULL, &graphics_pipeline);
	if(r != VK_SUCCESS) return;

	target->framebuffer_count = target->swapchain_image_count;
	target->framebuffers = malloc(sizeof(VkFramebuffer) * target->framebuffer_count);
	for(int i = 0; i < target->framebuffer_count; i++) {
		VkImageView attachments[] = {
			target->swapchain_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = target->swapchain_extent.width;
		framebuffer_info.height = target->swapchain_extent.height;
		framebuffer_info.layers = 1;

		vkCreateFramebuffer(context->vulkan.selected_device.logical_device,
		                    &framebuffer_info, NULL, &target->framebuffers[i]);
	}


	VkCommandPool cmd_pool;
	VkCommandPoolCreateInfo cmd_pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmd_pool_info.queueFamilyIndex = context->vulkan.selected_device.queue_indices[GRAPHICS_QUEUE_INDEX];
	
	vkCreateCommandPool(context->vulkan.selected_device.logical_device,
	                    &cmd_pool_info, NULL, &cmd_pool);
	
	VkCommandBuffer cmd_buffer;
	VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	cmd_buffer_alloc_info.commandPool = cmd_pool;
	cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_buffer_alloc_info.commandBufferCount = 1;

	r = vkAllocateCommandBuffers(context->vulkan.selected_device.logical_device, 
	                            &cmd_buffer_alloc_info, &cmd_buffer);
	if(r != VK_SUCCESS) return;

	r = vkResetCommandBuffer(cmd_buffer, 0);
	if(r != VK_SUCCESS) return;

	VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = NULL;

	r = vkBeginCommandBuffer(cmd_buffer, &begin_info);
	if(r != VK_SUCCESS) return;
	VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	render_pass_begin_info.renderPass = render_pass;
	render_pass_begin_info.framebuffer = target->framebuffers[image_index];
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = target->swapchain_extent;

	VkClearValue clear_value = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_value;


	vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
	vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
	vkCmdSetScissor(cmd_buffer, 0, 1, &scissor_rect);

	vkCmdDraw(cmd_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(cmd_buffer);

	vkEndCommandBuffer(cmd_buffer);

	VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	VkSemaphore wait_semaphores[] = {image_avaliable_semaphore};
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;

	VkSemaphore signal_semaphores[] = {render_finished_semaphore};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;
	vkQueueSubmit(context->vulkan.selected_device.queues[GRAPHICS_QUEUE_INDEX],
	              1, &submit_info, in_flight_fence);
	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	VkSwapchainKHR swapchains[] = { target->swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = NULL;

	vkQueuePresentKHR(context->vulkan.selected_device.queues[GRAPHICS_QUEUE_INDEX], &present_info);
}