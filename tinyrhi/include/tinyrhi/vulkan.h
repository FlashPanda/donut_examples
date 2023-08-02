#pragma once
#include "vulkan/vulkan_core.h"
#include "tinyrhi/vulkan-device.h"
#include "vulkan/vulkan_win32.h"

namespace tinyrhi::vulkan
{
	bool initVulkan();

	// We need a window to connect to swap chain, which we will create with glfw.
	bool createWindow();

	bool createSwapChain();

	void createCommandBuffers();

	VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);


	void destroySwapChain();
}