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

		/** Default command pool for the graphics queue family index */
		VkCommandPool commandPool = VK_NULL_HANDLE;

		/** Indicates queue family indices */
		struct QueueFamilyIndices {
			uint32_t graphics;
			uint32_t compute;
			uint32_t transfer;
		} queueFamilyIndices;


		VulkanDevice(VkPhysicalDevice inPhysicalDevice);
		~VulkanDevice();

		operator VkDevice() const
		{
			return logicalDevice;
		}

		/**
		 * Get the index of a queue family that supports the requested queue flags
		 */
		uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

		/**
		* Check if an extension is supported by the (physical device)
		*
		* @param extension Name of the extension to check
		*
		* @return True if the extension is supported (present in the list read at device creation time)
		*/
		bool VulkanDevice::extensionSupported(std::string extension)
		{
			return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
		}
	};
}