#include "tinyrhi/vulkan-device.h"
#include <cassert>

tinyrhi::vulkan::VulkanDevice::VulkanDevice(VkPhysicalDevice inPhysicalDevice)
{
	assert(inPhysicalDevice);
	physicalDevice = inPhysicalDevice;

	// Store device properties which also contain limits and sparse properties
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	// Store feature properties of the physical device
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);

	// Store memory properties of the physical device
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	// Queue family properties, used for setting up requested queues upon device creation
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// Get list of supported extensions
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
	if (extCount > 0) {
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extensions.data())) {
			for (auto ext : extensions) {
				supportedExtensions.push_back(ext.extensionName);
			}
		}
	}


	/** Create Logical device */
	// By now we just create a device with all avaialble extensions and features
	//std::vector<VkDeviceQueueCreateInfo

}

tinyrhi::vulkan::VulkanDevice::~VulkanDevice()
{

}

