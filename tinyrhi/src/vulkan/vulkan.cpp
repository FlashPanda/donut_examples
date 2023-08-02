#include "tinyrhi/vulkan.h"
#include <string>
#include <vector>
#include <windows.h>
#include <iostream>
#include "tinyrhi/vulkan-swapchain.h"
#include "GLFW/glfw3.h"

namespace tinyrhi::vulkan
{
	/** Settings that can be changed e.g. by command line argument */
	struct Settings {
		/** Activates validation layers (and message output) when set to true */
		bool validation = true;
		/** fullscreen mode */
		bool fullscreen = false;
		/** v-sync will be forced if it is set to true */
		bool vsync = false;
		/** Enable UI overlay */
		bool overlay = true;
	}settings;




	std::string title = "Vulkan Example";
	std::string name = "triangle";

	uint32_t apiVersion = VK_API_VERSION_1_0;

	std::vector<std::string> supportedInstanceExtensions;
	std::vector<const char*> enabledInstanceExtensions;		// extensions used in this example

	// Vulkan instance, stores all per-application states
	VkInstance instance;
	// Physical device (GPU) that Vulkan will use
	VkPhysicalDevice physicalDevice;
	// Stores physical device properties (for e.g. checking device limits)
	VkPhysicalDeviceProperties deviceProperties;
	// Stores the features available on the selected physical device (for e.g. checking if a feature is available)
	VkPhysicalDeviceFeatures deviceFeatures;
	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

	// Device abstraction. Typically a logical device, with physical device infomation in it.
	tinyrhi::vulkan::VulkanDevice* vulkanDevice;

	/** Logical device, application's view of the physical device (GPU) */
	VkDevice device;

	// Handle to the device graphics queue that command buffers are submitted to
	VkQueue queue;

	// Depth buffer format (selected during Vulkan initialization)
	VkFormat depthFormat;

	// Wraps the swap chain to present images (framebuffers) to the windowing system
	VulkanSwapChain swapChain;

	GLFWwindow* m_Window = nullptr;

	// Command buffers used for rendering
	std::vector<VkCommandBuffer> drawCmdBuffers;
}

bool tinyrhi::vulkan::initVulkan()
{
	VkResult err;

	// create Vulkan instance

	// application info
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = name.c_str();
	appInfo.pEngineName = name.c_str();
	appInfo.apiVersion = apiVersion;

	// instance extensions
	std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Enable surface extensions depending on os
#if defined(_WIN32)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

	// query extensions supported by the instance and store for later use
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

	// Enabled requested instance extensions
	if (enabledInstanceExtensions.size() > 0)
	{
		for (const char* enabledExtension : enabledInstanceExtensions)
		{
			// check if requested extension is available
			if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end())
			{
				std::cerr << "Required instance extension \"" << enabledExtension << "\" is not present at instance level!" << std::endl;
			}
			else {
				instanceExtensions.push_back(enabledExtension);
			}
		}
	}

	// create info
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	// TODO: Enable the debug utils extension if supportd


	if (instanceExtensions.size() > 0)
	{
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	// Note: VK_LAYER_KHRONOS_validation contains all current validation functionality.
	// Insatance layer property
	const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
	if (settings.validation)
	{
		// Check if this layer is available at instance level
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);	// enum layer count first.
		std::vector<VkLayerProperties> instancelayerProperties(instanceLayerCount);	// retain property space
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instancelayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties layer : instancelayerProperties) {
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
	err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

	// TODO: set up debug functions
	if (err != VK_SUCCESS) return false;
	// ~create Vulkan instance

	// TODO: debugging request

	/** Create logical device */
	// Physical device count
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	{
		VkResult result = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Failed to enumerate physical devices!" << std::endl;
			return false;
		}
	}
	if (gpuCount == 0) {
		std::cerr << "No device with Vulkan support found" << std::endl;
		return false;
	}

	// Get physical devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
	if (err != VK_SUCCESS) {
		std::cerr << "Can not get physical devices' info." << std::endl;
		return false;
	}

	// we will use this first physical device
	physicalDevice = physicalDevices[0];

	// Get device properties
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	// Get device features
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	// Get device memory properties
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	// Create Vulkan device. This is an abstraction of device, which can be used for hide device creation.
	// We can use Vulkan to create device, as well as DirextX 12.
	vulkanDevice = new tinyrhi::vulkan::VulkanDevice(physicalDevice);

	/** ~Create Logical device */

	device = vulkanDevice->logicalDevice;

	vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

	// Find a suitable depth and/or stencial format
	VkBool32 validFormat{ false };
	validFormat = getSupportedDepthFormat(physicalDevice, depthFormat);
	assert(validFormat);

	swapChain.set(instance, physicalDevice, device);

	// create window
	createWindow();

	// create swap chain
	createSwapChain();

	return true;
}

VkBool32 tinyrhi::vulkan::getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> formatList = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : formatList)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = format;
			return true;
		}
	}

	return false;
}

static void ErrorCallback_GLFW(int error, const char* description)
{
	fprintf(stderr, "GLFW error: %s\n", description);
	exit(1);
}

bool tinyrhi::vulkan::createWindow()
{
	if (!glfwInit())
	{
		return false;
	}

	glfwSetErrorCallback(ErrorCallback_GLFW);

	glfwDefaultWindowHints();

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	glfwWindowHint(GLFW_SAMPLES, 1);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);   // Ignored for fullscreen

	m_Window = glfwCreateWindow(1280, 720,
		"Basic triangle ",
		nullptr,
		nullptr);


	if (m_Window == nullptr)
	{
		return false;
	}

	//glfwSetWindowUserPointer(m_Window, this);
}

bool tinyrhi::vulkan::createSwapChain()
{

	swapChain.initSurface(m_Window);

	uint32_t width = 1280;
	uint32_t height = 720;
	swapChain.create(&width, &height, false, false);

	glfwShowWindow(m_Window);

	// TODO: update window size
	// Acturally, window operations are not swap chain related operations, 
	// So, consider moving it into another function.
}

void tinyrhi::vulkan::destroySwapChain()
{
	if (vulkanDevice)
		vkDeviceWaitIdle(*vulkanDevice);

	if (swapChain)
	{
		vkDestroySwapchainKHR(*vulkanDevice, swapChain, nullptr);
	}
}

void tinyrhi::vulkan::createCommandBuffers()
{

}