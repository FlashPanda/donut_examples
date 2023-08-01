#include "tinyrhi/vulkan-swapchain.h"
#include "GLFW/glfw3.h"

void tinyrhi::vulkan::VulkanSwapChain::initSurface(GLFWwindow* glfwWindow)
{
	const VkResult res = glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface);
	if (res != VK_SUCCESS)
	{
		return;
	}

	// Get avariable queue family properties
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
	assert(queueCount >= 1);

	std::vector<VkQueueFamilyProperties> queueProps(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

	// Iterate over each queue to learn whether it supports presenting:
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; ++i) {
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);
	}

	// Search for a grphics and a presnet queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; ++i) {
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphicsQueueNodeIndex == UINT32_MAX) {
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	if (presentQueueNodeIndex == UINT32_MAX) {
		// There's no queue that supports both present and graphics
		// try to find a separate present queue
		for (uint32_t i = 0; i < queueCount; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Exit if either a graphics or a presenting queue hasn't been found
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
		std::cout << "Could not find a graphics and/or presenting queue!" << std::endl;
		return;
	}

	queueNodeIndex = graphicsQueueNodeIndex;

	// Get list of supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	assert(formatCount > 0);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

	// We want to get a format that best suits our needs, so we try to get one from a set of preferred formats
	// Initialize the format to the first one returned by the implementation in case we can't find one of the preferred formats
	VkSurfaceFormatKHR selectedFormat = surfaceFormats[0];
	std::vector<VkFormat> preferredImageFormats = {
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_A8B8G8R8_UNORM_PACK32
	};

	for (auto& availableFormat : surfaceFormats) {
		if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) {
			selectedFormat = availableFormat;
			break;
		}
	}

	colorFormat = selectedFormat.format;
	colorSpace = selectedFormat.colorSpace;
}

void tinyrhi::vulkan::VulkanSwapChain::set(VkInstance _instance, VkPhysicalDevice _physicalDevice, VkDevice _device)
{
	instance = _instance;
	physicalDevice = _physicalDevice;
	device = _device;
}

// Create the swapchain and get its images with given width and height
void tinyrhi::vulkan::VulkanSwapChain::create(uint32_t* width, uint32_t* height, bool vsync /*= false*/, bool fullscreen /*= false*/)
{
	// Store the current swap chain handle so we can use it later on to ease up recreation
	VkSwapchainKHR oldSwapChain = swapChain;

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps;
	assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps) == VK_SUCCESS);

	// Get available present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	assert(presentModeCount > 0);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF,
	// the size of the surface will be set by swapchain
	if (surfCaps.currentExtent.width == (uint32_t)-1) {
		// if the surface size is undefined, the size is set to 
		// the size of the images requested.
		swapchainExtent.width = *width;
		swapchainExtent.height = *height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCaps.currentExtent;
		*width = surfCaps.currentExtent.width;
		*height = surfCaps.currentExtent.height;
	}

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
}

