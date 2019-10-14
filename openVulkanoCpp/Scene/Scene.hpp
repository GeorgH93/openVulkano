#pragma once
#include "Node.hpp"
#include "Camera.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		struct Scene : virtual public IInitable, virtual public ICloseable
		{
			Node* root;
			std::vector<Drawable*> shapeList;
			Shader* shader;
			Camera* camera;

		public:
			Scene() : root(nullptr) {}

			virtual ~Scene()
			{
				if (root) Scene::Close();
			}

			void Init() override
			{
				Node* newRoot = new Node();
				newRoot->Init();
				Init(newRoot);
			}

			void Init(Node* root)
			{
				if (root->GetParent()) throw std::runtime_error("Node has a parent! Only nodes without a parent may be a root node!");
				root->SetScene(this);
				root->SetParent(root);
				this->root = root;
			}

			void Close() override
			{
				//TODO
			}

			Node* GetRoot() const
			{
				return root;
			}

			void RegisterDrawable(Drawable* drawable)
			{
				if (drawable->GetScene() != this) drawable->SetScene(this);
				if (Utils::Contains(shapeList, drawable)) return; // Prevent duplicate entries
				shapeList.push_back(drawable);
			}

			void RemoveDrawable(Drawable* drawable)
			{
				Utils::Remove(shapeList, drawable);
				drawable->SetScene(nullptr);
			}

			void SetCamera(Camera* camera)
			{
				this->camera = camera;
			}

			Camera* GetCamera() const
			{
				return camera;
			}
			
			/**
			 * \brief Checks if the scene is valid and attempts to fix problems.
			 */
			void Validate()
			{
				for (Drawable* drawable : shapeList)
				{
					if(drawable->GetScene() != this)
					{
						if (!drawable->GetScene()) drawable->SetScene(this);
						else Logger::SCENE->error("Scene is linked with drawable from different scene!!!"); //TODO handle
					}
				}
				//TODO check node tree
			}
		};
	}
}
