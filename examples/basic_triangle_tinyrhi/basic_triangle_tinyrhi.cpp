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

class DeviceManager_Vulkan
{
public:
	GLFWwindow* window = nullptr;
	const int32_t WIDTH = 1280;
	const int32_t HEIGHT = 720;

public:
    void InitWindow();
    void InitVulkan();
    void MainLoop();

    void Cleanup();

    void createInstance();

    bool checkValidationLayerSupport();

    std::vector<const char*> getRequiredExtensions();

private:
    VkInstance instance;
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
}

void DeviceManager_Vulkan::MainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void DeviceManager_Vulkan::Cleanup()
{
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
	instanceCreateInfo.enabledExtensionCount = extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    // validation layers, controlled by a boolean variant.
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount = extensions.size();
    }
    else
    {
    }
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;

    // We can create instance now!
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vulkan instance!");
    }

    // Enumerate insatance extensions.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    // Output to log
    donut::log::info("available extensions");
    for (const auto& ext : extensions)
    {
        donut::log::info(ext.extensionName);
    }
}

bool DeviceManager_Vulkan::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

}

std::vector<const char*> DeviceManager_Vulkan::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

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
