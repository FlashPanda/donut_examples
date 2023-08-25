/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#include <donut/app/ApplicationBase.h>
#include <donut/engine/ShaderFactory.h>
#include <donut/app/DeviceManager.h>
#include <donut/core/log.h>
#include <donut/core/vfs/VFS.h>
#include <nvrhi/utils.h>
//#include <tinyrhi/vulkan.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <set>
#include <fstream>
#include <donut/core/math/math.h>
#include <iostream>
#if defined(_WIN32)
#include <vulkan/vulkan_win32.h>
#endif

#ifndef VK_CHECK_RESULT
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res)																				\
	{																									\
		std::cerr << "Fatal : VkResult is not success in " << __FILE__ << " at line " << __LINE__ << "\n"; \
	}																									\
}
#endif

class DeviceManager_Vulkan
{
public:
    bool initVulkan()
    {
		VkResult err;

		// Vulkan instance
		err = createInstance(settings.validation);
		if (err) {
			return false;
		}

		// Physical device
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
		if (gpuCount == 0)
		{
			std::cerr << "No device with vulkan support found" << std::endl;
			return false;
		}

		// Enumerate devices
		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
		if (err)
		{
			std::cerr << "Could not enumerate physical devices " << std::endl;
			return false;
		}

		// GPU selection

		// Select physical device to be used for the Vulkan example
		// Defaults to the first device unless specified by parameters e.g. command line
		uint32_t selectedDevice = 0;
		physicalDevice = physicalDevices[selectedDevice];

		// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
		
		// Vulkan device creation
		// This is handled by a separate class that gets a logical device representation
		// and encapsulates functions related to a device

		// Queue family properties, used for setting up requested queues upon device creation
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		// Get list of supported extensions (device side, distinguished from instance extensions)
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (auto ext : extensions)
				{
					supportedExtensions.push_back(ext.extensionName);
				}
			}
		}


        return true;
    }

    HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
    {

    }

    virtual void prepare()
    {

    }

    void renderLoop()
    {

    }

	//void getEnabledFeatures() {
	//}

	/** @brief (Virtual) Creates the application wide Vulkan instance */
    virtual VkResult createInstance(bool enableValidation) 
    {
        settings.validation = enableValidation;

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name.c_str();
        appInfo.pEngineName = name.c_str();
        appInfo.apiVersion = apiVersion;

        std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
    
#if defined (_WIN32)
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        // check extension properties
        // save supported extensions which will be used later
        uint32_t extCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (VkExtensionProperties extension : extensions)
				{
					supportedInstanceExtensions.push_back(extension.extensionName);
				}
			}
		}

        // enable requested instance extensions
        if (enabledInstanceExtensions.size() > 0)
        {
			for (const char* enabledExtension : enabledInstanceExtensions)
			{
				// Output message if requested extension is not available
				if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end())
				{
					std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
				}
				instanceExtensions.push_back(enabledExtension);
			}
        }

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = NULL;
		instanceCreateInfo.pApplicationInfo = &appInfo;

		// Enable the debug utils extension if available (e.g. when debugging tools are present)
		if (settings.validation || std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Specify the extensions to be used in creating the information.
		if (instanceExtensions.size() > 0)
		{
			instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
			instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
		}

		// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
		const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
		if (settings.validation)
		{
			// Check if this layer is available at instance level
			uint32_t instanceLayerCount;
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
			std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
			bool validationLayerPresent = false;
			for (VkLayerProperties layer : instanceLayerProperties) {
				if (strcmp(layer.layerName, validationLayerName) == 0) {
					validationLayerPresent = true;
					break;
				}
			}
			if (validationLayerPresent) {
				instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
				instanceCreateInfo.enabledLayerCount = 1;
			}
			else {
				std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
			}
		}

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

		return result;
    }

	/**
	 * Create the logical device based on the assigned physical device, also gets default queue family indices
	 */
	VkResult        createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky,
		// especially if the application requests different queue types

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlay depending on the implementation
		const float defaultQueuePriority(0.f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
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

		// device extensions
		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (useSwapChain)
		{
			// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		// device create info
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		// If a pNext(Chain) has been passed, we need to add it to the device creation info
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		if (pNextChain)
		{
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		this->enabledFeatures = enabledFeatures;

		VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
		if (result != VK_SUCCESS)
		{
			return result;
		}

		// Create a default command pool for graphics command buffers
		commandPool = createCommandPool(queueFamilyIndices.graphics);

		return result;

	}

	// Get the index of a queue family that supports the requested queue flags
	uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const
	{
		// Try to find a queue family index that supports compute but not graphics
		if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i)
			{
				if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		// Try to find a queue family index that supports transfer but not graphics and compute
		if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); ++i)
			{
				if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
					((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
					((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				return i;
			}
		}

		throw std::runtime_error("Could not find a matching queue family index");
	}

	/**
	* Create a command pool for allocation command buffers from
	*
	* @param queueFamilyIndex Family index of the queue to create the command pool for
	* @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
	*
	* @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
	*
	* @return A handle to the created command buffer
	*/
	VkCommandPool   createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		VK_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
		return cmdPool;
	}
public:
	/** @brief Example settings that can be changed e.g. by command line arguments */
	struct Settings {
		/** @brief Activates validation layers (and message output) when set to true */
		bool validation = false;
		/** @brief Set to true if fullscreen mode has been requested via command line */
		bool fullscreen = false;
		/** @brief Set to true if v-sync will be forced for the swapchain */
		bool vsync = false;
		/** @brief Enable UI overlay */
		bool overlay = true;
	} settings;

	std::string name = "HelloTriangle";
	uint32_t apiVersion = VK_API_VERSION_1_0;

    std::vector<std::string> supportedInstanceExtensions;
	std::vector<const char*> enabledInstanceExtensions;

	// Vulkan instance, stores all per-application states
	VkInstance instance;
	// Physical device (GPU) that Vulkan will use
	VkPhysicalDevice physicalDevice;
	/** @brief Logical device representation (application's view of the device) */
	VkDevice logicalDevice;
	// Stores physical device properties (for e.g. checking device limits)
	VkPhysicalDeviceProperties deviceProperties;
	// Stores the features available on the selected physical device (for e.g. checking if a feature is available)
	VkPhysicalDeviceFeatures deviceFeatures;
	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	/** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
	VkPhysicalDeviceFeatures enabledFeatures{};

	/* in device */
	/** @brief Queue family properties of the physical device */
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	/** @brief List of extensions supported by the device */
	std::vector<std::string> supportedExtensions;
	/** @brief Contains queue family indices */
	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} queueFamilyIndices;
	/** @brief Default command pool for the graphics queue family index */
	VkCommandPool commandPool = VK_NULL_HANDLE;
};

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int __argc, const char** __argv)
#endif
{
    // 一、实例
    // 实例可以看成是Vulkan API本身
    // 涉及概念：Instance，实例扩展、层

    // 二、逻辑设备
    // 逻辑设备可以看成是我们操作的物理设备的一个程序抽象，我们的操作只能对逻辑设备进行操作
    // 创建逻辑设备需要用到物理设备，所以需要查询物理设备的属性，而查询则需要上一步中的实例。
    // 涉及概念：物理设备（设备属性、功能、内存属性、队列族、设备扩展）

    // 命令池，命令队列
    return 0;
}
