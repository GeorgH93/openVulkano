#pragma once
#include <atomic>
#include <vector>

namespace openVulkanoCpp
{
	namespace Data
	{
		template <class T>
		class ReadOnlyAtomicArrayQueue final
		{
			T* data;
			std::atomic<size_t> size;

		public:
			ReadOnlyAtomicArrayQueue(std::vector<T>& data)
			{
				this->data = data.data();
				size.store(data.size());
			}

			ReadOnlyAtomicArrayQueue(T* data, size_t size)
			{
				this->data = data;
				this->size.store(size);
			}

			~ReadOnlyAtomicArrayQueue() = default;

			size_t GetSize() const
			{
				return size.load(std::memory_order_relaxed);
			}

			T* Pop()
			{
				size_t s = size.load(std::memory_order_relaxed);
				while (size > 0 && !size.compare_exchange_weak(s, s - 1));
				if (s > 0) return &data[s - 1];
				return nullptr;
			}
		};
	}
}