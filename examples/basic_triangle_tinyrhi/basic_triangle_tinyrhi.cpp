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
    // ��ʼ��
    glfwInit();

    // GLFWԭ��ΪΪ��OpenGL��Ƶģ�������Ҫ��ʾ��������֯����OpenGL������
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // ��ֹ���ڴ�С�ĸı�
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // �������ڣ���ô��ھ��
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
    // ���ٴ���
    glfwDestroyWindow(window);

    // ��ֹglfw
    glfwTerminate();
}

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int __argc, const char** __argv)
#endif
{
    // һ��ʵ��
    // ʵ�����Կ�����Vulkan API����
    // �漰���Instance��ʵ����չ����

    // �����߼��豸
    // �߼��豸���Կ��������ǲ����������豸��һ������������ǵĲ���ֻ�ܶ��߼��豸���в���
    // �����߼��豸��Ҫ�õ������豸��������Ҫ��ѯ�����豸�����ԣ�����ѯ����Ҫ��һ���е�ʵ����
    // �漰��������豸���豸���ԡ����ܡ��ڴ����ԡ������塢�豸��չ��

    // ����أ��������

    DeviceManager_Vulkan device;
	
    device.InitWindow();

    device.InitVulkan();

    device.MainLoop();

    device.Cleanup();
    return 0;
}
