#pragma once
#define _USE_MATH_DEFINES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Node.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		class Camera : public Node
		{
		protected:
			float nearPlane, farPlane;
			float width, height;
		public:
			glm::mat4x4 projection, view, viewProjection;

			Camera() = default;
			virtual ~Camera() = default;

		public:
			void Init(float width, float height, float nearPlane, float farPlane)
			{
				this->width = width;
				this->height = height;
				this->nearPlane = nearPlane;
				this->farPlane = farPlane;
				Node::Init();
				UpdateProjectionMatrix();
			}

			virtual void SetSize(const float& width, const float& height)
			{
				this->width = width;
				this->height = height;
				UpdateProjectionMatrix();
			}

			void SetNearPlane(float nearPlane)
			{
				this->nearPlane = nearPlane;
			}

			void SetFarPlane(float farPlane)
			{
				this->farPlane = farPlane;
			}


			float NearPlane() const
			{
				return nearPlane;
			}

			float FarPlane() const
			{
				return farPlane;
			}

			virtual void UpdateProjectionMatrix() = 0;

			void UpdateViewProjectionMatrix()
			{ // In vulkan the screen space is defined as y=0=top and y=1=bottom and thus the coordinate have to be flipped
				viewProjection = projection * glm::mat4x4(1,0,0,0,0,-1,0,0,0,0,1,0,0,0,0,1) * view;
			}

			void UpdateWorldMatrix(const glm::mat4x4& parentWorldMat) override
			{
				Node::UpdateWorldMatrix(parentWorldMat);
				view = glm::inverse(GetWorldMatrix());
				UpdateViewProjectionMatrix();
			}

			const glm::mat4x4& GetViewProjectionMatrix() const
			{
				return viewProjection;
			}

			const glm::mat4x4* GetViewProjectionMatrixPointer() const
			{
				return &viewProjection;
			}
		};

		class PerspectiveCamera : public Camera
		{
		protected:
			float fov, aspect;

		public:
			void Init(float fovDegrees, float width, float height, float nearPlane, float farPlane)
			{
				this->fov = glm::radians(fovDegrees);
				aspect = width / height;
				Camera::Init(width, height, nearPlane, farPlane);
			}

			void SetSize(const float& width, const float& height) override
			{
				aspect = width / height;
				Camera::SetSize(width, height);
			}

			void SetAspect(const float& aspect)
			{
				this->aspect = aspect;
				Camera::SetSize(aspect, 1);
			}

			void SetFovX(const float& fov)
			{
				SetFov(2 * atan(tan(fov * 0.5f) * aspect));
			}

			void SetFovXRad(const float& fov)
			{
				SetFovRad(2 * atan(tan(fov * 0.5f) * aspect));
			}

			void SetFov(const float& fov)
			{
				SetFovRad(glm::radians(fov));
			}

			void SetFovRad(const float& fov)
			{
				this->fov = fov;
			}

			float GetFov() const
			{
				return glm::degrees(fov);
			}

			float GetFovX() const
			{
				return 2 * atan(tan(GetFov() * 0.5f) * aspect);
			}

			float GetFovRad() const
			{
				return fov;
			}

			float GetFovXRad() const
			{
				return 2 * atan(tan(fov * 0.5f) * aspect);
			}

			void UpdateProjectionMatrix() override
			{
				projection = glm::perspectiveLH_ZO(fov, aspect, nearPlane, farPlane);
				UpdateViewProjectionMatrix();
			}
		};

		class OrthographicCamera : public Camera
		{
		public:
			void UpdateProjectionMatrix() override
			{
				const float widthHalf = width * 0.5f, heightHalf = height * 0.5f;
				projection = glm::orthoLH_ZO(-widthHalf, widthHalf, -heightHalf, heightHalf, nearPlane, farPlane);
				UpdateViewProjectionMatrix();
			}
		};
	}
}
