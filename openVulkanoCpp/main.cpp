#include "Host/GraphicsAppManager.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Shader.hpp"
#include "Base/EngineConfiguration.hpp"

using namespace openVulkanoCpp::Scene;

uint32_t GEOS = 3000, OBJECTS = 10000, DYNAMIC = 1000;

class ExampleApp : public openVulkanoCpp::IGraphicsApp
{
	Scene scene;
	PerspectiveCamera cam;
	Material mat;
	Shader shader;
	std::vector<Drawable*> drawablesPool;
	std::vector<Node*> nodesPool;

public:
	std::string GetAppName() override { return "ExampleApp"; }
	std::string GetAppVersion() override { return "v1.0"; }
	int GetAppVersionAsInt() override { return 1; }

	void Init() override
	{
		std::srand(1);
		scene.Init();
		cam.Init(70, 16, 9, 0.1f, 100);
		scene.SetCamera(&cam);
		cam.SetMatrix(glm::translate(glm::mat4(1), glm::vec3(0,0,-10)));
		shader.Init("Shader\\basic", "Shader\\basic");
		drawablesPool.resize(GEOS);
		for(int i = 0; i < GEOS; i++)
		{
			Geometry* geo = new Geometry();
			geo->InitCube(std::rand() % 1000 / 1000.0f + 0.01f, std::rand() % 1000 / 1000.0f + 0.01f, std::rand() % 1000 / 1000.0f + 0.01f, glm::vec4((std::rand() % 255) / 255.0f, (std::rand() % 255) / 255.0f, (std::rand() % 255) / 255.0f, 1));
			drawablesPool[i] = new Drawable();
			drawablesPool[i]->Init(geo, &mat);
		}
		nodesPool.resize(OBJECTS);
		for(int i = 0; i < OBJECTS; i++)
		{
			nodesPool[i] = new Node();
			nodesPool[i]->Init();
			scene.GetRoot()->AddChild(nodesPool[i]);
			if (i < DYNAMIC) nodesPool[i]->SetUpdateFrequency(UpdateFrequency::Always);
			nodesPool[i]->AddDrawable(drawablesPool[std::rand() % GEOS]);
			nodesPool[i]->SetMatrix(glm::translate(glm::mat4x4(1), glm::vec3((std::rand() % 10000) / 1000.0f - 5, (std::rand() % 10000) / 1000.0f - 5, (std::rand() % 10000) / 1000.0f - 5)));
		}
		
		scene.shader = &shader;

		GetGraphicsAppManager()->GetRenderer()->SetScene(&scene);
	}

	void Tick() override
	{
		for(int i = 0; i < DYNAMIC; i++)
		{
			nodesPool[i]->SetMatrix(glm::translate(glm::mat4x4(1), glm::vec3((std::rand() % 10000) / 1000.0f - 5, (std::rand() % 10000) / 1000.0f - 5, (std::rand() % 10000) / 1000.0f - 5)));
		}
	}

	void Close() override{}
};

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
	std::cout << "Amount of Threads to use [2]: ";
	int threads = 2;
	std::string input;
	std::getline(std::cin, input);
	if (!input.empty())
	{
		std::istringstream stream(input);
		stream >> threads;
	}
	std::cout << "Amount of geometries to produce [" << GEOS << "]: ";
	std::getline(std::cin, input);
	if (!input.empty())
	{
		std::istringstream stream(input);
		stream >> GEOS;
	}
	std::cout << "Amount of objects to render [" << OBJECTS << "]: ";
	std::getline(std::cin, input);
	if (!input.empty())
	{
		std::istringstream stream(input);
		stream >> OBJECTS;
	}
	std::cout << "Amount of moving objects [" << DYNAMIC << "]: ";
	std::getline(std::cin, input);
	if (!input.empty())
	{
		std::istringstream stream(input);
		stream >> DYNAMIC;
	}
	DYNAMIC = std::min(DYNAMIC, OBJECTS);
	openVulkanoCpp::EngineConfiguration::GetEngineConfiguration()->SetNumThreads(threads);
	openVulkanoCpp::IGraphicsAppManager* manager = new openVulkanoCpp::GraphicsAppManager(new ExampleApp());
	manager->Run();
	return 0;
}
