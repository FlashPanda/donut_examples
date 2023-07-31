#include "tinyrhi/vulkan-device.h"
#include <cassert>
#include <stdexcept>

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
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extensions.data()) == VK_SUCCESS) {
			for (auto ext : extensions) {
				supportedExtensions.push_back(ext.extensionName);
			}
		}
	}


	/** Create Logical device */
	{
		// By now we just create a device with all avaialble extensions and features
		VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlap depending on the implementation

		const float defaultQueuePriority(0.f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else {
			queueFamilyIndices.graphics = 0;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
			{
				// If transfer family index differs, we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.transfer = queueFamilyIndices.graphics;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
			{
				// If transfer family index differs, we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.transfer = queueFamilyIndices.graphics;
		}

		// Create the logical device representation
		std::vector<const char*> deviceExtensions;
		// We need swap chain.
		deviceExtensions.push_back("VK_KHR_swapchain");

		// Device create info
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		VkPhysicalDeviceFeatures enabledFeatures{};
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		VkPhysicalDeviceFeatures2 physicalDeviceFeature2{};

		if (deviceExtensions.size() > 0)
		{
			for (const char* ex : deviceExtensions) {
				if (!extensionSupported(ex)) {
					std::cerr << "Enabled device extension \"" << ex << "\" is not present at device level\n";
				}
			}
			deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->enabledFeatures = enabledFeatures;

		VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
		if (result != VK_SUCCESS)
			return;

		// create command pool
		VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
		cmdPoolInfo.flags = createFlags;
		
		vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &commandPool);
	}

	

}

tinyrhi::vulkan::VulkanDevice::~VulkanDevice()
{

}

uint32_t tinyrhi::vulkan::VulkanDevice::getQueueFamilyIndex(VkQueueFlags queueFlags) const
{
	// Dedicated compute queue
	// Try to find a queue family index that supports compute but not graphics
	if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i) {
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				return i;
			}
		}
	}

	// Dedicated queue for transfer
	// which means the queue family supports transfer but not graphics and compute
	if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i) {
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
				((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				return i;
			}
		}
	}

	// for other queue types or if no separate compute queue is present, return first one to 
	// support the requested flags
	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
			return i;
		}
	}

	throw std::runtime_error("Could not find a matching queue family index");
}

