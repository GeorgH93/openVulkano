#pragma once
#include <vulkan/vulkan.hpp>
#include "../Device.hpp"
#include "../../Scene/Shader.hpp"
#include "../../Scene/Vertex.hpp"
#include "../../Base/ICloseable.hpp"
#include "../Resources/IShaderOwner.hpp"
#include "IRecordable.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		vk::PrimitiveTopology ToVkTopology(Scene::Topology topology)
		{
			switch (topology) {
			case Scene::Topology::PointList: return vk::PrimitiveTopology::ePointList;
				case Scene::Topology::LineList: return vk::PrimitiveTopology::eLineList;
				case Scene::Topology::LineStripe: return vk::PrimitiveTopology::eLineStrip;
				case Scene::Topology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
				case Scene::Topology::TriangleStripe: return vk::PrimitiveTopology::eTriangleStrip;
				default: throw std::runtime_error("Unknown topology!");
			}
		}
		
		struct VulkanShader : virtual public ICloseable, virtual public IRecordable
		{
			Scene::Shader* shader = nullptr;
			vk::Device device;
			vk::ShaderModule shaderModuleVertex, shaderModuleFragment;
			vk::Pipeline pipeline;
			IShaderOwner* owner;

			VulkanShader() = default;
			virtual ~VulkanShader() { if (shader) VulkanShader::Close(); }

			void Init(Context* context, Scene::Shader* shader, IShaderOwner* owner)
			{
				this->device = context->device->device;
				this->shader = shader;
				this->owner = owner;
				shaderModuleVertex = context->device->CreateShaderModule(shader->vertexShaderName + ".vert.spv");
				shaderModuleFragment = context->device->CreateShaderModule(shader->fragmentShaderName + ".frag.spv");
				std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos(2);
				shaderStageCreateInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shaderModuleVertex, "main" };
				shaderStageCreateInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shaderModuleFragment, "main" };
				
				auto vertexBindDesc = vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
				std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
				attributeDescriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position));
				attributeDescriptions.emplace_back(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal));
				attributeDescriptions.emplace_back(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent));
				attributeDescriptions.emplace_back(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, biTangent));
				attributeDescriptions.emplace_back(4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, textureCoordinates));
				attributeDescriptions.emplace_back(5, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color));

				auto viewport = context->swapChain.GetFullscreenViewport();
				auto scissor = context->swapChain.GetFullscreenScissor();
				vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = { {}, 1, &viewport, 1, &scissor };
				vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { {}, 1, &vertexBindDesc,
					static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data() };
				vk::PipelineInputAssemblyStateCreateInfo inputAssembly = { {}, ToVkTopology(shader->topology), 0 };
				vk::PipelineRasterizationStateCreateInfo rasterizer = {};
				rasterizer.cullMode = vk::CullModeFlagBits::eBack;
				vk::PipelineMultisampleStateCreateInfo msaa = {};
				vk::PipelineDepthStencilStateCreateInfo depth = { {}, 1, 1, vk::CompareOp::eGreater };
				vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
				colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR;
				vk::PipelineColorBlendStateCreateInfo colorInfo = {};
				colorInfo.attachmentCount = 1;
				colorInfo.pAttachments = &colorBlendAttachment;

				
				
				vk::GraphicsPipelineCreateInfo pipelineCreateInfo = { {}, static_cast<uint32_t>(shaderStageCreateInfos.size()), shaderStageCreateInfos.data(), &pipelineVertexInputStateCreateInfo, &inputAssembly,
				nullptr, &viewportStateCreateInfo, &rasterizer, &msaa, &depth, &colorInfo, nullptr, context->pipeline.pipelineLayout, context->swapChainRenderPass.renderPass };
				pipeline = this->device.createGraphicsPipeline({}, pipelineCreateInfo);
				
			}

			void Record(vk::CommandBuffer& cmdBuffer, uint32_t bufferId) override
			{
				cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
			}

			void Close() override
			{
				owner->RemoveShader(this);
				shader = nullptr;
				device.destroyPipeline(pipeline);
				device.destroyShaderModule(shaderModuleVertex);
				device.destroyShaderModule(shaderModuleFragment);
			}
		};

	}
}
