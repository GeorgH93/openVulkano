#pragma once
#include <vector>
#include "../Base/ICloseable.hpp"
#include "Geometry.hpp"
#include "Material.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		class Node;
		class Scene;

		struct Drawable : virtual public ICloseable
		{
			std::vector<Node*> nodes;
			Scene* scene = nullptr;
			Geometry* mesh = nullptr;
			Material* material = nullptr;

		public:
			Drawable() = default;

			explicit Drawable(const Drawable* toCopy)
			{
				mesh = toCopy->mesh;
				material = toCopy->material;
			}

			virtual ~Drawable()
			{
				if(mesh) Drawable::Close();
			}

			Drawable* Copy() const
			{
				return new Drawable(this);
			}

			void Init(Geometry* mesh, Material* material)
			{
				if (this->mesh || this->material) throw std::runtime_error("Drawable is already initialized.");
				this->mesh = mesh;
				this->material = material;
			}

			void Init(Drawable* drawable)
			{
				if (mesh || material) throw std::runtime_error("Drawable is already initialized.");
				this->mesh = drawable->mesh;
				this->material = drawable->material;
			}

			void Close() override
			{
				if (!nodes.empty()) throw std::runtime_error("Drawable is still being used!!!");
				mesh = nullptr;
				material = nullptr;
			}

			Scene* GetScene() const
			{
				return scene;
			}

		private:
			friend class Node;
			friend class Scene;

			void AddNode(Node* node)
			{
				if (!mesh) throw std::runtime_error("Drawable is not initialized.");
				if (Utils::Contains(nodes, node)) throw std::runtime_error("A drawable must not use the same node more than once.");
				nodes.push_back(node);
			}

			void SetScene(Scene* scene);

			void RemoveNode(Node* node);
		};
	}
}
