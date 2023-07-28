#pragma once
#include "vulkan/vulkan_core.h"
#include <vector>
#include <string>


namespace tinyrhi::vulkan
{
	struct VulkanDevice
	{
		/** Physical device representation */
		VkPhysicalDevice physicalDevice;
		/** Logical device representation */
		VkDevice logicalDevice;
		/** Properties of the physical device */
		VkPhysicalDeviceProperties properties;
		/** Features of the physical device */
		VkPhysicalDeviceFeatures features;
		/** Features that are enabled for use on the physical device */
		VkPhysicalDeviceFeatures enabledFeatures;
		/** Memory properties and heaps info of the physical device */
		VkPhysicalDeviceMemoryProperties memoryProperties;
		/** Queue family properties of the physical device */
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		/** extensions supported by the device, similar to instance extensions */
		std::vector<std::string> supportedExtensions;

		VulkanDevice(VkPhysicalDevice inPhysicalDevice);
		~VulkanDevice();

		operator VkDevice() const
		{
			return logicalDevice;
		}
	};
}