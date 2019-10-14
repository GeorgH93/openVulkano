# openVulkano
openVulkano is a simple Vulkan rendering engine which is capable of recording command buffers from multiple threads.

## Building
### Visual Studio
#### Requierements
* Visual Studio 2019, 2017 might work but not tested
* [Vulkan SDK v1.1.x](https://vulkan.lunarg.com/sdk/home#windows)
* [Assimp SDK v4.1](https://github.com/assimp/assimp/releases/download/v4.1.0/assimp-sdk-4.1.0-setup.exe)
* git

#### Build it
1. Clone it `git clone --recursive https://github.com/GeorgH93/openVulkano`
2. Open the `openVulkanoCpp.sln` with Visual Studio
3. If you are not using the default path of Assimp change it in the project properties
4. Use Visual Studio to build or run it

