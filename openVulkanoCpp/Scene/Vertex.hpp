#pragma once
#include <glm/glm.hpp>
#include <assimp/vector2.h>
#include <assimp/vector3.h>

namespace openVulkanoCpp
{
	struct Vertex
	{
		glm::vec3 position, normal, tangent, biTangent, textureCoordinates;
		glm::vec4 color;

		Vertex() = default;

		Vertex(const aiVector3D& pos) : position(pos.x, pos.y, pos.z), normal(), tangent(), biTangent(), textureCoordinates(), color()
		{}

		Vertex(const float& x, const float& y, const float& z, const float& nx, const float& ny, const float& nz, const float& u, const float& v)
			: position({ x, y, z }), normal({ nx, ny, nz }), tangent(), biTangent(), textureCoordinates({ u, v, 0 }), color()
		{}

		Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec3& biTangent, const glm::vec2& textureCoordinates)
			: position(position), normal(normal), tangent(tangent), biTangent(biTangent), textureCoordinates(textureCoordinates, 0), color()
		{}

		Vertex(const glm::vec3 & position, const glm::vec3 & normal, const glm::vec3 & tangent, const glm::vec3 & biTangent, const glm::vec3 & textureCoordinates)
			: position(position), normal(normal), tangent(tangent), biTangent(biTangent), textureCoordinates(textureCoordinates), color()
		{}

		~Vertex() = default;

		void Set(const float& x, const float& y, const float& z)
		{
			position = { x, y, z };
		}

		void Set(const glm::vec3& position)
		{
			this->position = position;
		}

		void Set(const aiVector3D& position)
		{
			this->position = { position.x, position.y, position.z };
		}

		void Set(const float& x, const float& y, const float& z, const float& nx, const float& ny, const float& nz, const float& u, const float& v)
		{
			this->position = { x, y, z };
			this->normal = { nx, ny, nz };
			this->textureCoordinates = { u, v, 0 };
		}

		void Set(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoordinates)
		{
			this->position = position;
			this->normal = normal;
			this->textureCoordinates = { textureCoordinates, 0 };
		}

		void Set(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec3& biTangent, const glm::vec2& textureCoordinates)
		{
			this->position = position;
			this->normal = normal;
			this->tangent = tangent;
			this->biTangent = biTangent;
			this->textureCoordinates = { textureCoordinates,0 };
		}

		void SetNormal(const float& nx, const float& ny, const float& nz)
		{
			this->normal = { nx, ny, nz };
		}

		void SetNormal(const glm::vec3& normal)
		{
			this->normal = normal;
		}

		void SetNormal(const aiVector3D& normal)
		{
			this->normal = { normal.x, normal.y, normal.z };
		}

		void SetTangent(const float& tx, const float& ty, const float& tz)
		{
			this->tangent = { tx, ty, tz };
		}

		void SetTangent(const glm::vec3& tangent)
		{
			this->tangent = tangent;
		}

		void SetTangent(const aiVector3D& tangent)
		{
			this->tangent = { tangent.x, tangent.y, tangent.z };
		}

		void SetTangentAndBiTangent(const float& tx, const float& ty, const float& tz, const float& bx, const float& by, const float& bz)
		{
			this->biTangent = { bx, by, bz };
			this->tangent = { tx, ty, tz };
		}

		void SetTangentAndBiTangent(const glm::vec3& tangent, const glm::vec3& biTangent)
		{
			this->biTangent = biTangent;
			this->tangent = tangent;
		}

		void SetTangentAndBiTangent(const aiVector3D& tangent, const aiVector3D& biTangent)
		{
			this->tangent = { tangent.x, tangent.y, tangent.z };
			this->biTangent = { biTangent.x, biTangent.y, biTangent.z };
		}

		void SetBiTangent(const float& bx, const float& by, const float& bz)
		{
			this->biTangent = { bx, by, bz };
		}

		void SetBiTangent(const glm::vec3& biTangent)
		{
			this->biTangent = biTangent;
		}

		void SetBiTangent(const aiVector3D& biTangent)
		{
			this->biTangent = { biTangent.x, biTangent.y, biTangent.z };
		}

		void SetTextureCoordinates(const float& u, const float& v)
		{
			this->textureCoordinates = { u, v, 0 };
		}

		void SetTextureCoordinates(const float& u, const float& v, const float& w)
		{
			this->textureCoordinates = { u, v, w };
		}

		void SetTextureCoordinates(const glm::vec2& textureCoordinates)
		{
			this->textureCoordinates = { textureCoordinates, 0 };
		}

		void SetTextureCoordinates(const glm::vec3& textureCoordinates)
		{
			this->textureCoordinates = textureCoordinates;
		}

		void SetTextureCoordinates(const aiVector2D& textureCoordinates)
		{
			this->textureCoordinates = { textureCoordinates.x, textureCoordinates.y, 0 };
		}

		void SetTextureCoordinates(const aiVector3D& textureCoordinates)
		{
			this->textureCoordinates = { textureCoordinates.x, textureCoordinates.y, textureCoordinates.z };
		}

		void SetColor(const float& r, const float& g, const float& b, const float& a = 1)
		{
			color = { r,g,b,a };
		}

		void SetColor(const glm::vec4& color)
		{
			this->color = color;
		}

		void SetColor(const aiColor4D& color)
		{
			this->color = { color.r, color.g, color.b, color.a };
		}
	};
}
