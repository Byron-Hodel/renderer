#include "vulkan_renderpass.h"

int8_t vulkan_create_renderpass(const vulkan_context_t context, vulkan_renderpass_t* renderpass,
                                vec2_t pos, vec2_t extents, float depth, uint32_t stencil)
{
	const uint32_t attachment_count = 2;
	VkAttachmentDescription attachments[attachment_count];

	VkAttachmentDescription color_attachment = {0};
	color_attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0] = color_attachment;

	VkAttachmentReference color_ref;
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {0};
	depth_attachment.format = VK_FORMAT_D32_SFLOAT;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1] = depth_attachment;

	VkAttachmentReference depth_ref;
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_ref;

	VkRenderPassCreateInfo renderpass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderpass_info.attachmentCount = attachment_count;
	renderpass_info.pAttachments = attachments;
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpass;
	renderpass_info.dependencyCount = 1;
	renderpass_info.pDependencies = &dependency;

	VkResult r = vkCreateRenderPass(context.selected_device.handle, &renderpass_info, NULL, &renderpass->handle);
	if(r != VK_SUCCESS) return 0;

	renderpass->pos = pos;
	renderpass->extents = extents;
	renderpass->depth = depth;
	renderpass->stencil = stencil;
	return 1;
}

void vulkan_destroy_renderpass(const vulkan_context_t context, vulkan_renderpass_t* renderpass) {
	vkDestroyRenderPass(context.selected_device.handle, renderpass->handle, NULL);
	*renderpass = (vulkan_renderpass_t) {0};
}

void vulkan_renderpass_begin(vulkan_renderpass_t* renderpass, vulkan_command_buffer_t* buffer, vulkan_framebuffer_t* framebuffer) {
	VkClearValue clear_colors[2];
	clear_colors[0] = (VkClearValue) {{{0.0f, 0.0f, 1.0f, 1.0f}}};
	clear_colors[1] = (VkClearValue) {{{0.0f, 0.0f, 1.0f, 1.0f}}};
	VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
	begin_info.renderPass = renderpass->handle;
	begin_info.renderArea.offset.x = renderpass->pos.x;
	begin_info.renderArea.offset.y = renderpass->pos.y;
	begin_info.renderArea.extent.width = renderpass->extents.x;
	begin_info.renderArea.extent.height = renderpass->extents.y;
	begin_info.clearValueCount = 2;
	begin_info.pClearValues = clear_colors;
	begin_info.framebuffer = framebuffer->handle;
	vkCmdBeginRenderPass(buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan_renderpass_end(vulkan_renderpass_t* renderpass, vulkan_command_buffer_t* buffer) {
	vkCmdEndRenderPass(buffer->handle);
}