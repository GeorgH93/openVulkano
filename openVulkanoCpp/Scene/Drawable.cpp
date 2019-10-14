#include "Drawable.hpp"
#include "Scene.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		void Drawable::SetScene(Scene* scene)
		{
			if (this->scene == scene) return;
			if (scene && this->scene) throw std::runtime_error("Drawable has been associated with a scene already!");
			this->scene = scene;
			if(scene) scene->RegisterDrawable(this);
		}

		void Drawable::RemoveNode(Node* node)
		{
			Utils::Remove(nodes, node);
			if (nodes.empty())
			{
				scene = nullptr;
				scene->RemoveDrawable(this);
			}
		}
	}
}