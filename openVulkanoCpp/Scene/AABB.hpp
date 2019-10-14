#pragma once
#include <glm/glm.hpp>
#include "../Base/IInitable.hpp"

namespace openVulkanoCpp
{
	namespace Scene
	{
		/**
		 * \brief A class that represents an axis aligned bounding box
		 */
		class AABB final : public virtual IInitable
		{
			glm::vec3 min, max;

		public:
			AABB() : min(INFINITY), max(-INFINITY) {}
			~AABB() = default;

			/**
			 * \brief Initiates the AABB to min=Inf, max=-Inf
			 */
			void Init() override
			{
				min = glm::vec3(INFINITY);
				max = glm::vec3(-INFINITY);
			}

			/**
			 * \brief Initiates the AABB to a single point (min=max=point)
			 * \param point The point that should be used as min and max of the AABB
			 */
			void Init(const glm::vec3& point)
			{
				min = max = point;
			}

			/**
			 * \brief Initiates the AABB from some other AABB
			 * \param other The other AABB that should be copied
			 */
			void Init(const AABB& other)
			{
				min = other.GetMin();
				max = other.GetMax();
			}

			const glm::vec3& GetMin() const { return min; }

			const glm::vec3& GetMax() const { return max; }

			void Grow(const glm::vec3& point)
			{
				min = glm::min(min, point);
				max = glm::max(max, point);
			}

			void Grow(const AABB& otherAABB)
			{
				min = glm::min(min, otherAABB.GetMin());
				max = glm::max(max, otherAABB.GetMax());
			}

			void Grow(const AABB& otherAABB, glm::mat4x4 transformation)
			{
				//TODO
			}

			glm::vec3 GetDiagonal() const
			{
				return max - min;
			}

			glm::vec3 GetCenter() const
			{
				return min + (GetDiagonal() * 0.5f);
			}

			/**
			 * \brief Checks if the AABB overlaps with an other AABB
			 * \param other The other AABB that should be checked
			 * \return true if the AABB overlaps with the other, false if not
			 */
			bool IsOverlapping(const AABB& other) const
			{
				return !(other.min.x > max.x || other.max.x < min.x || other.min.y > max.y || other.max.y < min.y || other.min.z > max.z || other.max.z < min.z);
			}

			/**
			 * \brief Resets the AABB to min=Inf, max=-Inf, same as Init()
			 */
			void Reset()
			{
				Init();
			}
		};
	}
}
