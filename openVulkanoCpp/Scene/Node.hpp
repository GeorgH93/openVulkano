#pragma once
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>
#include "../Base/Utils.hpp"
#include "../Base/IInitable.hpp"
#include "../Base/ICloseable.hpp"
#include "Drawable.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		class Scene;

		enum class UpdateFrequency
		{
			Always, Sometimes, Never
		};
		
		struct Node : virtual IInitable, virtual ICloseable
		{
			friend Scene;
		protected:
			static const glm::mat4x4 IDENTITY;
		public:
			glm::mat4x4 localMat, worldMat;
			bool enabled = true;
			Node* parent = nullptr;
			Scene* scene = nullptr;
			std::vector<Node*> children;
			std::vector<Drawable*> drawables;
			UpdateFrequency matrixUpdateFrequency = UpdateFrequency::Never;
			ICloseable* renderNode = nullptr;

		public:
			Node() = default;
			virtual ~Node() = default;

			void Init() override
			{
				if (parent || scene || !children.empty() || !drawables.empty()) throw std::runtime_error("Node already initialized");
				localMat = worldMat = IDENTITY;
				enabled = true;
				parent = nullptr;
				children = std::vector<Node*>();
				drawables = std::vector<Drawable*>();
			}

			void Close() override
			{
				children.clear();
				if (renderNode) renderNode->Close();
				parent = nullptr;
				scene = nullptr;
				enabled = false;
				if (!children.empty()) Logger::SCENE->warn("Closing Node that has children!");
				for (Node* child : children)
				{
					child->SetParent(nullptr);
				}
				children.clear();
				for(size_t i = drawables.size(); i > 0; i--)
				{
					RemoveDrawable(drawables[i]);
				}
			}

			void AddChild(Node* node)
			{
				node->SetParent(this);
				children.push_back(node);
				node->UpdateWorldMatrix(worldMat);
			}

			void AddChild(Drawable* drawable)
			{
				AddDrawable(drawable);
			}

			void RemoveChild(Node* node)
			{
				if (node->parent == this)
				{
					Utils::Remove(children, node);
					node->SetParent(nullptr);
				}
			}

			void RemoveChild(Drawable* drawable)
			{
				RemoveDrawable(drawable);
			}

			void AddDrawable(Drawable* drawable)
			{
				if (scene) drawable->SetScene(scene);
				else if (drawable->GetScene()) Logger::SCENE->warn("Drawable is already associated with a scene, but the node it was added to is not!");
				drawable->AddNode(this);
				drawables.push_back(drawable);
			}

			void RemoveDrawable(Drawable* drawable)
			{
				drawable->RemoveNode(this);
				Utils::Remove(drawables, drawable);
			}

			void SetMatrix(glm::mat4x4 mat)
			{
				localMat = mat;
				UpdateWorldMatrix(parent ? parent->GetWorldMatrix() : IDENTITY);
			}

			const glm::mat4x4& GetMatrix() const
			{
				return localMat;
			}

			const glm::mat4x4& GetWorldMatrix() const
			{
				return worldMat;
			}

			bool IsEnabled() const
			{
				return enabled;
			}

			void Enable()
			{
				enabled = true;
			}

			void Disable()
			{
				enabled = false;
			}

			Node* GetParent() const
			{
				return parent;
			}

			Scene* GetScene() const
			{
				return scene;
			}

			bool IsRoot() const
			{
				return scene && parent == this;
			}

			UpdateFrequency GetUpdateFrequency()
			{
				return matrixUpdateFrequency;
			}

			void SetUpdateFrequency(UpdateFrequency frequency)
			{
				if (!children.empty()) throw std::runtime_error("The update must not be changed for nodes with children.");
				this->matrixUpdateFrequency = frequency;
			}

		protected:
			virtual void UpdateWorldMatrix(const glm::mat4x4& parentWorldMat)
			{
				worldMat = parentWorldMat * localMat;
				for (const auto& node : children)
				{
					node->UpdateWorldMatrix(worldMat);
				}
			}

		private:
			void SetParent(Node* parent)
			{
				if (this->parent && parent) throw std::runtime_error("Node already has a parent! Nodes must not be used multiple times!");
				this->parent = parent;
				if(parent && parent != this) this->scene = parent->scene;
				if (!parent) SetScene(nullptr);
			}

			void SetScene(Scene* scene)
			{
				if (this->scene && scene) throw std::runtime_error("Node already has a scene!");
				this->scene = scene;
				for (const auto& node : children)
				{
					node->SetScene(scene);
				}
				if (scene)
				{
					for (size_t i = 0; i < drawables.size(); i++)
					{
						Scene* drawableScene = drawables[i]->GetScene();
						if(drawableScene)
						{
							if(drawableScene != scene)
							{
								Logger::SCENE->warn("Drawable is already associated with a scene! Creating copy.");
								drawables[i] = drawables[i]->Copy();
							}
						}
						drawables[i]->SetScene(scene);
					}
				}
			}
		};
	}
}
