#include "tinyrhi/vulkan.h"
#include "vulkan/vulkan_core.h"
#include <string>
#include <vector>
#include "vulkan/vulkan_win32.h"

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

	// ~create Vulkan instance
}

