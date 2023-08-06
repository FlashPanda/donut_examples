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
    // 销毁窗口
    glfwDestroyWindow(window);

    // 终止glfw
    glfwTerminate();
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
