#pragma once
#include "vulkan/vulkan_core.h"
#include "tinyrhi/vulkan-device.h"
#include "vulkan/vulkan_win32.h"

namespace tinyrhi::vulkan
{
	bool initVulkan();

	VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);
}