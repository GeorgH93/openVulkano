#pragma once
#include <string>
#include <stdexcept>
#include "../Base/ICloseable.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		enum class Topology
		{
			PointList, LineList, LineStripe, TriangleList, TriangleStripe
		};
		
		struct Shader : public virtual ICloseable
		{
			std::string vertexShaderName, fragmentShaderName;
			Topology topology = Topology::TriangleList;
			ICloseable* renderShader = nullptr;

			Shader() = default;
			~Shader() { if (renderShader) Shader::Close(); }

			void Init(const std::string& vertexShaderName, const std::string& fragmentShaderName)
			{
				if (renderShader) throw std::runtime_error("Shader already initialized!");
				this->vertexShaderName = vertexShaderName;
				this->fragmentShaderName = fragmentShaderName;
			}

			void Close() override
			{
				renderShader->Close();
				renderShader = nullptr;
			}
		};
	}
}
