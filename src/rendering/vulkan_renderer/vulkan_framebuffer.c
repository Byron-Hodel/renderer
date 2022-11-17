#include "vulkan_framebuffer.h"
#include <malloc.h>

int8_t vulkan_framebuffer_create(const vulkan_context_t context, vulkan_framebuffer_t* framebuffer,
                                 vulkan_renderpass_t* renderpass, const uint32_t width, const uint32_t height,
                                 uint32_t attachment_count, VkImageView* attachments)
{
	VkImageView* new_attachments = calloc(attachment_count, sizeof(VkImageView));
	for(uint32_t i = 0; i < attachment_count; i++) new_attachments[i] = attachments[i];

	VkFramebufferCreateInfo framebuffer_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	framebuffer_info.renderPass = renderpass->handle;
	framebuffer_info.attachmentCount = attachment_count;
	framebuffer_info.pAttachments = new_attachments;
	framebuffer_info.width = width;
	framebuffer_info.height = height;
	framebuffer_info.layers = 1;

	VkResult r = vkCreateFramebuffer(context.selected_device.handle, &framebuffer_info, NULL, &framebuffer->handle);
	if(r != VK_SUCCESS) {
		return 0;
	}

	framebuffer->width = width;
	framebuffer->height = height;
	framebuffer->attachment_count = attachment_count;
	framebuffer->attachments = new_attachments;
	framebuffer->renderpass = renderpass;
	return 1;
}

void vulkan_framebuffer_destroy(const vulkan_context_t context, vulkan_framebuffer_t* framebuffer) {
	vkDestroyFramebuffer(context.selected_device.handle, framebuffer->handle, NULL);
	free(framebuffer->attachments);
	framebuffer->width = 0;
	framebuffer->height = 0;
	framebuffer->attachment_count = 0;
	framebuffer->attachments = NULL;
	framebuffer->renderpass = NULL;
}