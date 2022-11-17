#include "vulkan_pipeline.h"

int8_t vulkan_create_graphics_pipeline(const vulkan_context_t* context, vulkan_graphics_pipeline_t* pipeline,
                                       graphics_pipeline_create_info_t create_info)
{
	VkResult r;
	VkShaderModuleCreateInfo vert_shader_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	vert_shader_info.codeSize = create_info.vert_shader_src_len;
	vert_shader_info.pCode = (uint32_t*)create_info.vert_shader_src;

	VkShaderModuleCreateInfo frag_shader_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	frag_shader_info.codeSize = create_info.frag_shader_src_len;
	frag_shader_info.pCode = (uint32_t*)create_info.frag_shader_src;

	VkShaderModule vert_shader_module, frag_shader_module;
	r = vkCreateShaderModule(context->selected_device.handle, &vert_shader_info, NULL, &vert_shader_module);
	if(r != VK_SUCCESS) return 0;

	r = vkCreateShaderModule(context->selected_device.handle, &frag_shader_info, NULL, &frag_shader_module);
	if(r != VK_SUCCESS) {
		vkDestroyShaderModule(context->selected_device.handle, vert_shader_module, NULL);
		return 0;
	}

	VkPipelineShaderStageCreateInfo shader_stages[2];
	VkPipelineShaderStageCreateInfo vert_stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage.module = vert_shader_module;
	vert_stage.pName = "main";
	shader_stages[0] = vert_stage;

	VkPipelineShaderStageCreateInfo frag_stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage.module = frag_shader_module;
	frag_stage.pName = "main";
	shader_stages[1] = frag_stage;


	VkPipelineVertexInputStateCreateInfo vert_input_state = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkViewport viewport;
	VkPipelineViewportStateCreateInfo viewport_state = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;


	VkPipelineRasterizationStateCreateInfo rasterization_state = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	//rasterization_state.rasterizerDiscardEnable = VK_TRUE;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterization_state.lineWidth = 1.0f;


	VkPipelineMultisampleStateCreateInfo multisample_state = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state.minSampleShading = 1.0f;

	VkPipelineColorBlendAttachmentState color_blend_attachment;
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;


	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	depth_stencil_state.depthTestEnable = 0;
	depth_stencil_state.depthWriteEnable = 0;


	VkPipelineColorBlendStateCreateInfo color_blend_state = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	color_blend_state.attachmentCount = 1;
	color_blend_state.pAttachments = &color_blend_attachment;


	VkDynamicState dynamic_states[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_info = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	dynamic_state_info.dynamicStateCount = 2;
	dynamic_state_info.pDynamicStates = dynamic_states;


	VkPipelineLayout layout;
	VkPipelineLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

	r = vkCreatePipelineLayout(context->selected_device.handle, &layout_info, NULL, &layout);
	if(r != VK_SUCCESS) {
		vkDestroyShaderModule(context->selected_device.handle, vert_shader_module, NULL);
		vkDestroyShaderModule(context->selected_device.handle, frag_shader_module, NULL);
		return 0;
	}


	VkGraphicsPipelineCreateInfo pipeline_info = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vert_input_state;
	pipeline_info.pInputAssemblyState = &input_assembly_state;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterization_state;
	pipeline_info.pMultisampleState = &multisample_state;
	pipeline_info.pDepthStencilState = &depth_stencil_state;
	pipeline_info.pColorBlendState = &color_blend_state;
	pipeline_info.pDynamicState = &dynamic_state_info;
	pipeline_info.layout = layout;
	pipeline_info.renderPass = context->default_renderpass.handle;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineIndex = -1;
	// TODO: Maybe use basePipelineHandle at some point

	r = vkCreateGraphicsPipelines(context->selected_device.handle, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->handle);
	if(r != VK_SUCCESS) {
		vkDestroyShaderModule(context->selected_device.handle, vert_shader_module, NULL);
		vkDestroyShaderModule(context->selected_device.handle, frag_shader_module, NULL);
		vkDestroyPipelineLayout(context->selected_device.handle, layout, NULL);
		return 0;
	}

	pipeline->vert_shader_module = vert_shader_module;
	pipeline->frag_shader_module = frag_shader_module;
	pipeline->layout = layout;

	return 1;
}

void vulkan_destroy_graphics_pipeline(const vulkan_context_t* context, vulkan_graphics_pipeline_t* pipeline) {
	vkDestroyShaderModule(context->selected_device.handle, pipeline->vert_shader_module, NULL);
	vkDestroyShaderModule(context->selected_device.handle, pipeline->frag_shader_module, NULL);
	vkDestroyPipelineLayout(context->selected_device.handle, pipeline->layout, NULL);
	vkDestroyPipeline(context->selected_device.handle, pipeline->handle, NULL);
	pipeline->handle = VK_NULL_HANDLE;
	pipeline->vert_shader_module = VK_NULL_HANDLE;
	pipeline->frag_shader_module = VK_NULL_HANDLE;
	pipeline->layout = VK_NULL_HANDLE;
}