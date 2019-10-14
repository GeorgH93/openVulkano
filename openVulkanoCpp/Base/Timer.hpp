#pragma once
#include <chrono>
#include <cstdint>

namespace openVulkanoCpp
{
	/**
	 * \brief High-res timer
	 */
	class Timer
	{ //TODO maybe add a Windows option that uses QPC https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
		std::chrono::high_resolution_clock::time_point tPrev, tStop;
		int64_t tickNanoseconds, tickMilliseconds;
		uint64_t totalNanoseconds;
		double tickSeconds, totalSeconds;
		bool stopped;

	public:
		Timer()
		{
			Reset();
		}

		~Timer() = default;

		void Reset()
		{
			tickNanoseconds = 0;
			tickMilliseconds = 0;
			tickSeconds = 0;
			totalNanoseconds = 0;
			totalSeconds = 0;
			tPrev = std::chrono::high_resolution_clock::now();
			stopped = false;
		}

		void Start()
		{
			if (stopped)
			{
				tPrev += std::chrono::high_resolution_clock::now() - tStop;
				stopped = false;
			}
		}

		void Stop()
		{
			tStop = std::chrono::high_resolution_clock::now();
			stopped = true;
		}

		/**
		 * \brief Will update the timer
		 */
		void Tick()
		{
			if (stopped)
			{
				tickNanoseconds = 0;
			}
			else
			{
				const auto now = std::chrono::high_resolution_clock::now();
				tickNanoseconds = std::chrono::duration<int64_t, std::nano>(now - tPrev).count();
				tPrev = now;
				if (tickNanoseconds < 0) tickNanoseconds = 0;
			}
			totalNanoseconds += tickNanoseconds;
			tickMilliseconds = tickNanoseconds / 1000000;
			tickSeconds = tickNanoseconds / 1000000000.0;
			totalSeconds += tickSeconds;
		}

		int64_t GetTickNanoseconds() const { return tickNanoseconds; }

		int64_t GetTickMilliseconds() const { return tickMilliseconds; }

		double GetTickSeconds() const { return tickSeconds; }

		uint64_t GetTotalNanoseconds() const { return totalNanoseconds; }

		/**
		 * \brief Gets the total amount of seconds past since the timer has been started. This will drift over time!
		 * \return The summed total runtime of the timer.
		 */
		double GetTotalSeconds() const { return totalSeconds; }

		/**
		 * \brief Will recalculate the past time from the total nanoseconds and return it. This is more precise but also slower.
		 * \return The calculated total runtime of the timer.
		 */
		double GetTotalSecondsPrecise()
		{
			totalSeconds = totalNanoseconds / 1000000000.0;
			return totalSeconds;
		}
	};
}
