#pragma once
#include <stdexcept>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include "Vertex.hpp"
#include "../Base/Logger.hpp"
#include "../Base/Utils.hpp"
#include "../Base/ICloseable.hpp"
#include "AABB.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		enum class VertexIndexType
		{
			UINT16 = sizeof(uint16_t), UINT32 = sizeof(uint32_t)
		};

		struct Geometry : public virtual ICloseable
		{
			uint32_t vertexCount = 0, indexCount = 0;
			Vertex* vertices;
			void* indices;
			VertexIndexType indexType;
			AABB aabb;
			ICloseable* renderGeo = nullptr;

			Vertex* GetVertices() const { return vertices; }
			void* GetIndices() const { return indices; }
			uint16_t* GetIndices16() const { return static_cast<uint16_t*>(indices); }
			uint32_t* GetIndices32() const { return static_cast<uint32_t*>(indices); }
			uint32_t GetIndexCount() const { return indexCount; }
			uint32_t GetVertexCount() const { return vertexCount; }

			static Geometry* LoadFromFile(const std::string file)
			{
				Geometry* mesh = new Geometry();
				mesh->InitFromFile(file);
				return mesh;
			}

			Geometry() : vertexCount(0), indexCount(0), vertices(nullptr), indices(nullptr), indexType(VertexIndexType::UINT16) {}

			~Geometry()
			{
				if (vertices) Geometry::Close();
			}

			void InitFromFile(const std::string file)
			{
				Assimp::Importer importer;

				const uint32_t flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals |
					aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials | aiProcess_GenUVCoords | aiProcess_TransformUVCoords |
					aiProcess_ConvertToLeftHanded | aiProcess_PreTransformVertices | aiProcess_OptimizeGraph;

				const aiScene* scene = importer.ReadFile(file, flags);
				if (!scene) throw std::runtime_error("Failed to load file \"" + file + "\" Error: " + importer.GetErrorString());
				if (!scene->HasMeshes()) throw std::runtime_error("File \"" + file + "\" does not have any meshes");
				if (scene->mNumMeshes > 1) Logger::DATA->warn("File {0} contains more than one mesh. Only first one will be loaded", file);
				Init(scene->mMeshes[0]);
				importer.FreeScene();
			}

			/**
			 * \brief Creates the arrays for the vertices and indices. They will not be filled!
			 * \param vertexCount The amount of vertices that will be used
			 * \param indexCount The amount of indices that will be used
			 */
			void Init(uint32_t vertexCount, uint32_t indexCount)
			{
				if (this->vertexCount || this->indexCount) throw std::runtime_error("Geometry is already initialized.");
				this->vertexCount = vertexCount;
				this->indexCount = indexCount;
				indexType = (vertexCount > UINT16_MAX) ? VertexIndexType::UINT32 : VertexIndexType::UINT16;
				vertices = new Vertex[vertexCount];
				indices = malloc(static_cast<size_t>(Utils::EnumAsInt(indexType)) * indexCount);
				renderGeo = nullptr;
			}

			void Init(aiMesh* mesh)
			{
				aabb.Init();
				Init(mesh->mNumVertices, mesh->mNumFaces * 3); // Reserve the space for the data
				for (unsigned int i = 0; i < mesh->mNumVertices; i++)
				{
					vertices[i].Set(mesh->mVertices[i]);
					if (mesh->HasNormals()) vertices[i].SetNormal(mesh->mNormals[i]);
					if (mesh->HasTangentsAndBitangents())
					{
						vertices[i].SetTangentAndBiTangent(mesh->mTangents[i], mesh->mBitangents[i]);
					}
					if (mesh->HasTextureCoords(0)) vertices[i].SetTextureCoordinates(mesh->mTextureCoords[0][i]);
					if (mesh->HasVertexColors(0)) vertices[i].SetColor(mesh->mColors[0][i]);
					aabb.Grow(vertices[i].position);
				}
								
				for (unsigned int i = 0; i < mesh->mNumFaces; i++)
				{
					const aiFace face = mesh->mFaces[i];
					if (face.mNumIndices != 3) throw std::runtime_error("Mesh is not a triangle mesh!");
					for (unsigned int j = 0; j < face.mNumIndices; j++)
					{
						if (indexType == VertexIndexType::UINT16)
						{
							static_cast<uint16_t*>(indices)[i * face.mNumIndices + j] = static_cast<uint16_t>(face.mIndices[j]);
						}
						else
						{
							static_cast<uint32_t*>(indices)[i * face.mNumIndices + j] = face.mIndices[j];
						}
					}
				}

				//TODO load bones
				//TODO load materials
			}

			void InitCube(float x = 1, float y = 1, float z = 1, glm::vec4 color = glm::vec4(1))
			{
				Init(24, 36);
				SetIndices(new uint32_t[indexCount]{
					0, 1, 2, 0, 2, 3,		// front face index data
					4, 5, 6, 4, 6, 7,		// back face index data
					8, 9, 10, 8, 10, 11,	// top face index data
					12, 13, 14, 12, 14, 15,	// bottom face index data
					16, 17, 18, 16, 18, 19,	// left face index data
					20, 21, 22, 20, 22, 23	// right face index data
					}, indexCount);
				x *= 0.5f; y *= 0.5f; z *= 0.5f;
				int i = 0;
				// front face vertex data
				vertices[i++].Set(-x, +y, -z, +0, +0, -1, +0, +0);
				vertices[i++].Set(-x, -y, -z, +0, +0, -1, +0, +1);
				vertices[i++].Set(+x, -y, -z, +0, +0, -1, +1, +1);
				vertices[i++].Set(+x, +y, -z, +0, +0, -1, +1, +0);
				// back face vertex data
				vertices[i++].Set(-x, +y, +z, +0, +0, +1, +1, +0);
				vertices[i++].Set(+x, +y, +z, +0, +0, +1, +0, +0);
				vertices[i++].Set(+x, -y, +z, +0, +0, +1, +0, +1);
				vertices[i++].Set(-x, -y, +z, +0, +0, +1, +1, +1);
				// top face vertex data
				vertices[i++].Set(-x, -y, -z, +0, +1, +0, +0, +0);
				vertices[i++].Set(-x, -y, +z, +0, +1, +0, +0, +1);
				vertices[i++].Set(+x, -y, +z, +0, +1, +0, +1, +1);
				vertices[i++].Set(+x, -y, -z, +0, +1, +0, +1, +0);
				// bottom face vertex data
				vertices[i++].Set(-x, +y, -z, +0, -1, +0, +1, +0);
				vertices[i++].Set(+x, +y, -z, +0, -1, +0, +0, +0);
				vertices[i++].Set(+x, +y, +z, +0, -1, +0, +0, +1);
				vertices[i++].Set(-x, +y, +z, +0, -1, +0, +1, +1);
				// Fill in the left face vertex data
				vertices[i++].Set(-x, +y, +z, -1, +0, +0, +0, +0);
				vertices[i++].Set(-x, -y, +z, -1, +0, +0, +0, +1);
				vertices[i++].Set(-x, -y, -z, -1, +0, +0, +1, +1);
				vertices[i++].Set(-x, +y, -z, -1, +0, +0, +1, +0);
				// Fill in the right face vertex data
				vertices[i++].Set(+x, +y, -z, +1, +0, +0, +0, +0);
				vertices[i++].Set(+x, -y, -z, +1, +0, +0, +0, +1);
				vertices[i++].Set(+x, -y, +z, +1, +0, +0, +1, +1);
				vertices[i].Set(+x, +y, +z, +1, +0, +0, +1, +0);

				for(i = 0; i < vertexCount; i++)
				{
					vertices[i].color = color;
				}
			}

			void SetIndices(const uint32_t* data, uint32_t size, uint32_t offset = 0) const
			{
				size += offset;
				for(; offset < size; offset++)
				{
					if (indexType == VertexIndexType::UINT16)
					{
						static_cast<uint16_t*>(indices)[offset] = static_cast<uint16_t>(data[offset]);
					}
					else
					{
						static_cast<uint32_t*>(indices)[offset] = data[offset];
					}
				}
			}

			void Close() override
			{
				vertexCount = 0;
				indexCount = 0;
				Free();
				renderGeo->Close();
				renderGeo = nullptr;
			}

			void Free()
			{
				if(vertices) delete[] vertices;
				free(indices);
				vertices = nullptr;
				indices = nullptr;
			}
		};
	}
}
