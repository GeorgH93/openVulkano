#pragma once
#include <stdexcept>
#include <iostream>
#include <fstream>
#include "../Base/Render/IRenderer.hpp"
#include "../Base/UI/IWindow.hpp"
#include "../Base/Logger.hpp"
#include "Context.hpp"
#include "Resources/ResourceManager.hpp"
#include "../Data/ReadOnlyAtomicArrayQueue.hpp"
#include "CommandHelper.hpp"
#include "../Base/EngineConfiguration.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct WaitSemaphores
		{
			std::vector<vk::Semaphore> renderReady, renderComplete;
		};

		class Renderer : public IRenderer
		{
			Context context;
			std::shared_ptr<spdlog::logger> logger;
			std::vector<WaitSemaphores> waitSemaphores;
			Scene::Scene* scene = nullptr;
			std::ofstream perfFile;
			ResourceManager resourceManager;
			uint32_t currentImageId = -1;
			std::vector<std::thread> threadPool;
			std::vector<std::vector<CommandHelper>> commands;
			std::vector<std::vector<vk::CommandBuffer>> submitBuffers;
			VulkanShader* shader;

		public:
			Renderer() = default;
			virtual ~Renderer() = default;

			void Init(IGraphicsAppManager* graphicsAppManager, IWindow* window) override
			{
				logger = Logger::RENDER;
				logger->info("Initializing Vulkan renderer ...");
				IVulkanWindow* vulkanWindow = window->GetVulkanWindow();
				if (!vulkanWindow)
				{
					logger->error("The provided window is not compatible with Vulkan.");
					throw std::runtime_error("The provided window is not compatible with Vulkan.");
				}
				context.Init(graphicsAppManager, vulkanWindow);
				for (int i = 0; i < context.swapChain.GetImageCount(); i++)
				{
					waitSemaphores.emplace_back();
					waitSemaphores[i].renderComplete.push_back(context.device->device.createSemaphore({}));
					waitSemaphores[i].renderReady.resize(2);
				}
				resourceManager.Init(&context, context.swapChain.GetImageCount());
				threadPool.resize(EngineConfiguration::GetEngineConfiguration()->GetNumThreads() - 1);

				//Setup cmd pools and buffers
				commands.resize(threadPool.size() + 2); // One extra cmd object for the primary buffer and one for the main thread
				for(uint32_t i = 0; i < commands.size(); i++)
				{
					commands[i] = std::vector<CommandHelper>(context.swapChain.GetImageCount());
					for(size_t j = 0; j < commands[i].size(); j++)
					{
						commands[i][j].Init(context.device->device, context.device->queueIndices.GetGraphics(),
							(i == commands.size() - 1) ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary);
					}
				}
				submitBuffers.resize(context.swapChain.GetImageCount());
				for(uint32_t i = 0; i < submitBuffers.size(); i++)
				{
					submitBuffers[i].resize(commands.size() - 1);
					for (size_t j = 0; j < submitBuffers[i].size(); j++)
					{
						submitBuffers[i][j] = commands[j][i].cmdBuffer;
					}
				}

				shader = resourceManager.CreateShader(scene->shader);

				perfFile.open("perf.csv");
				perfFile << "sep=,\ntotal,fps\n";

				logger->info("Vulkan renderer initialized");
			}

			void Tick() override
			{
				currentImageId = context.swapChain.AcquireNextImage();
				auto tickStart= std::chrono::high_resolution_clock::now();

				Render();

				// Perf logging
				auto tickDone = std::chrono::high_resolution_clock::now();
				auto time = std::chrono::duration_cast<std::chrono::microseconds>(tickDone - tickStart).count();
				perfFile << time << ',' << 1000000000.0 / time << '\n';
			}

			void Close() override
			{
				perfFile.close();
				//context.Close();
			}

			std::string GetMainRenderDeviceName() override
			{
				return (context.device) ? context.device->GetDeviceName() : "Unknown";
			}

			void Resize(const uint32_t newWidth, const uint32_t newHeight) override
			{
				context.Resize(newWidth, newHeight);
				resourceManager.Resize();
			}

			void SetScene(Scene::Scene* scene) override
			{
				this->scene = scene;
			}

			Scene::Scene* GetScene() override
			{
				return scene;
			}

			CommandHelper* GetCommandData(uint32_t poolId)
			{
				return &commands[poolId][currentImageId];
			}

			static void RunThread(Renderer* renderer, Data::ReadOnlyAtomicArrayQueue<Scene::Drawable*>* jobQueue, uint32_t id)
			{
				renderer->RecordSecondaryBuffer(jobQueue, id);
			}

			void StartThreads(Data::ReadOnlyAtomicArrayQueue<Scene::Drawable*>* jobQueue)
			{
				for(uint32_t i = 0; i < threadPool.size(); i++)
				{
					threadPool[i] = std::thread(RunThread, this, jobQueue, i);
				}
			}

			void RecordPrimaryBuffer()
			{
				CommandHelper* cmdHelper = GetCommandData(commands.size() - 1);
				cmdHelper->Reset();
				cmdHelper->cmdBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
				context.swapChainRenderPass.Begin(cmdHelper->cmdBuffer);
			}

			void Submit()
			{
				for (auto& thread : threadPool) { thread.join(); } // Wait till everything is recorded
				CommandHelper* cmdHelper = GetCommandData(commands.size() - 1);
				cmdHelper->cmdBuffer.executeCommands(submitBuffers[currentImageId].size(), submitBuffers[currentImageId].data());
				context.swapChainRenderPass.End(cmdHelper->cmdBuffer);
				cmdHelper->cmdBuffer.end();
				std::array<vk::PipelineStageFlags, 2> stateFlags = { vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput), vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) };
				waitSemaphores[currentImageId].renderReady[0] = resourceManager.EndFrame();
				waitSemaphores[currentImageId].renderReady[1] = context.swapChain.imageAvailableSemaphore;
				vk::SubmitInfo si = vk::SubmitInfo(
					waitSemaphores[currentImageId].renderReady.size(), waitSemaphores[currentImageId].renderReady.data(), stateFlags.data(),
					1, &cmdHelper->cmdBuffer, 
					waitSemaphores[currentImageId].renderComplete.size(), waitSemaphores[currentImageId].renderComplete.data());
				context.device->graphicsQueue.submit(1, &si, context.swapChain.GetCurrentSubmitFence());
				context.swapChain.Present(context.device->graphicsQueue, waitSemaphores[currentImageId].renderComplete);
			}

			void Render()
			{
				resourceManager.StartFrame(currentImageId);
				Data::ReadOnlyAtomicArrayQueue<Scene::Drawable*> jobQueue(scene->shapeList);
				StartThreads(&jobQueue);
				RecordPrimaryBuffer();
				RecordSecondaryBuffer(&jobQueue, threadPool.size());
				Submit();
			}

			void RecordSecondaryBuffer(Data::ReadOnlyAtomicArrayQueue<Scene::Drawable*>* jobQueue, uint32_t poolId)
			{
				Scene::Geometry* lastGeo = nullptr;
				Scene::Node* lastNode = nullptr;
				CommandHelper* cmdHelper = GetCommandData(poolId);
				cmdHelper->Reset();
				vk::CommandBufferInheritanceInfo inheritance = { context.swapChainRenderPass.renderPass, 0, context.swapChainRenderPass.GetFrameBuffer()->GetCurrentFrameBuffer() };
				cmdHelper->cmdBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue, &inheritance });
				shader->Record(cmdHelper->cmdBuffer, currentImageId);
				cmdHelper->cmdBuffer.pushConstants(context.pipeline.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, 64, scene->GetCamera()->GetViewProjectionMatrixPointer());
				Scene::Drawable** drawablePointer;
				while((drawablePointer = jobQueue->Pop()) != nullptr)
				{
					Scene::Drawable* drawable = *drawablePointer;
					Scene::Geometry* mesh = drawable->mesh;
					if (mesh != lastGeo)
					{
						if (!mesh->renderGeo) resourceManager.PrepareGeometry(mesh);
						dynamic_cast<VulkanGeometry*>(mesh->renderGeo)->Record(cmdHelper->cmdBuffer, currentImageId);
						lastGeo = mesh;
					}
					for(Scene::Node* node : drawable->nodes)
					{
						if (node != lastNode)
						{
							if (!node->renderNode) resourceManager.PrepareNode(node);
							dynamic_cast<VulkanNode*>(node->renderNode)->Record(cmdHelper->cmdBuffer, currentImageId);
							lastNode = node;
						}
						cmdHelper->cmdBuffer.drawIndexed(mesh->GetIndexCount(), 1, 0, 0, 0);
					}
				}
				cmdHelper->cmdBuffer.end();
			}
		};
	}
}
