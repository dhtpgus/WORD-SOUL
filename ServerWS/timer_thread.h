#pragma once
#include <concurrent_priority_queue.h>
#include "session.h"
#include "free_list.h"

namespace timer {
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;

	struct Event {
		Event() = default;
		Event(int id, int milliseconds, Operation op)
			: id{ id }, tp{ Clock::now() + std::chrono::milliseconds{ milliseconds } }, op{ op } {}

		constexpr bool operator<(const Event& rhs) const {
			return tp > rhs.tp;
		}

		Clock::time_point tp{};
		int id{};
		Operation op{};
	};

	class EventPQ : public Concurrency::concurrent_priority_queue<Event> {
	public:
		void Emplace(int id, int milliseconds, Operation type) noexcept {
			push(Event{ id, milliseconds, type });
		}
	};

	inline EventPQ* event_pq;

	inline void Thread(HANDLE iocp, int id)
	{
		constexpr auto kSleep = std::chrono::milliseconds{ 2 };
		while (true) {
			Event event;
			auto current_tp = Clock::now();
			if (true == event_pq[id].try_pop(event)) {
				if (event.tp > current_tp) {
					event_pq[id].push(event);
					std::this_thread::sleep_for(kSleep);
					continue;
				}
				switch (event.op) {
				case Operation::kUpdateMobAI:
				{
					auto ox = free_list<OverEx>.Get(event.op);
					PostQueuedCompletionStatus(iocp, 1, event.id, &ox->over);
					break;
				}
				default:
					std::print("Unknown Operation: {}\n", static_cast<int>(event.op));
					system("pause");
					exit(1);
				}
				continue;
			}
			std::this_thread::sleep_for(kSleep);
		}
	}
}