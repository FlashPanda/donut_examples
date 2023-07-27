#include "tinyrhi/vulkan-device.h"

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
}

tinyrhi::vulkan::VulkanDevice::~VulkanDevice()
{

}

