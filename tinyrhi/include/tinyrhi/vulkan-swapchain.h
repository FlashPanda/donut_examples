#pragma once
#include "tinyrhi/vulkan.h"
namespace tinyrhi::vulkan
{
	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	class VulkanSwapChain{
	private:
		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;

	public:
		VkFormat colorFormat;
		VkColorSpaceKHR colorSpace;
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		uint32_t imageCount;
		std::vector<VkImage> images;
		std::vector<SwapChainBuffer> buffers;
		uint32_t queueNodeIndex = UINT32_MAX;

		void initSurface(class GLFWwindow* glfwWindow);

		// Set instance, physical and logical device to use for the swapchain and get all required function pointers
		void set(VkInstance _instance, VkPhysicalDevice _physicalDevice, VkDevice _device);

		void create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
	};
}