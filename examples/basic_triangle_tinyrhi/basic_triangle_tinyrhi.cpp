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

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else 
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Wrap Vk function
VkResult CreateDebugUtilsMessagerExt(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerExt(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

class DeviceManager_Vulkan
{
public:
	GLFWwindow* window = nullptr;
	const int32_t WIDTH = 1280;
	const int32_t HEIGHT = 720;
    VkDebugUtilsMessengerEXT debugMessenger;

public:
    void InitWindow();
    void InitVulkan();
    void MainLoop();

    void Cleanup();

    void createInstance();

    bool checkValidationLayerSupport();

    void setupDebugCallback() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        getDebugMessngerCreateInfo(createInfo);

        if (CreateDebugUtilsMessagerExt(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    std::vector<const char*> getRequiredExtensions();

    void getDebugMessngerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
		// debug messenger create info
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
    }

    // debug callback function
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        donut::log::error("validation layer: %s", pCallbackData->pMessage);

        return VK_FALSE;
    }

    void createDevice()
    {
        // The device we will create is logical device. In order to do it, 
        // we need to query the physical devices to pick a proper one.

        // pick a physical device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUS with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        assert(queueFamilyIndice != -1);

        /** 1. device queue */
        // We need create a device queue. 
        // So, we have to get queue family indice.
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndice;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.f;
        queueCreateInfo.pQueuePriorities = &queuePriority;  // We need to explicit set queue priority, no matter how many queues we create.

        /** 2. devie features */
        VkPhysicalDeviceFeatures deviceFeatures = {};

        /** Device create */
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // Set queue create infos. 1, by now.
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int32_t i = -1;
		for (const auto& queueFamily : queueFamilies) {
            ++i;
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndice = i;
                return true;
			}
		}
        
        return false;
    }
private:
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    int32_t queueFamilyIndice = -1;
};

void DeviceManager_Vulkan::InitWindow()
{
    // 初始化
    glfwInit();

    // GLFW原来为为了OpenGL设计的，所以需要显示的设置组织启动OpenGL上下文
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // 禁止窗口大小的改变
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // 创建窗口，获得窗口句柄
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Triangle", nullptr, nullptr);
}

void DeviceManager_Vulkan::InitVulkan()
{
    createInstance();

    setupDebugCallback();

    createDevice();
}

void DeviceManager_Vulkan::MainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void DeviceManager_Vulkan::Cleanup()
{
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerExt(instance, debugMessenger, nullptr);

    // destroy the instance
    vkDestroyInstance(instance, nullptr);

    // destroy the window
    glfwDestroyWindow(window);

    // glfw termination
    glfwTerminate();
}

void DeviceManager_Vulkan::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not avaliable!");
    }

    // App info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Instance create info
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;     // App info we created before.

    // Extensions
    auto extensions = getRequiredExtensions();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    // validation layers, controlled by a boolean variant.
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

        getDebugMessngerCreateInfo(debugCreateInfo);
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }


    // We can create instance now!
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vulkan instance!");
    }
}

bool DeviceManager_Vulkan::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayer(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayer.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayer)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char*> DeviceManager_Vulkan::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // if we need validation layers, we should make sure the debug extension is valid.
	if (enableValidationLayers) 
    {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

	return extensions;
}

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

    DeviceManager_Vulkan device;
	
    device.InitWindow();

    device.InitVulkan();

    device.MainLoop();

    device.Cleanup();
    return 0;
}
