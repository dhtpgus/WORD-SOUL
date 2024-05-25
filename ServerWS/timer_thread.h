#pragma once
#include <concurrent_priority_queue.h>
#include "session.h"
#include "free_list.h"

namespace timer {
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;

	enum class EventType {
		kNone, kUpdateMobAI
	};

	struct Event {
		Event() = default;
		Event(int id, int milliseconds, EventType type)
			: id{ id }, tp{ Clock::now() + std::chrono::milliseconds{ milliseconds } }, type{ type } {}

		constexpr bool operator<(const Event& rhs) const {
			return tp > rhs.tp;
		}

		Clock::time_point tp{};
		int id{};
		EventType type{};
	};

	class EventPQ : public Concurrency::concurrent_priority_queue<Event> {
	public:
		void Emplace(int id, int milliseconds, EventType type) noexcept {
			push(Event{ id, milliseconds, type });
		}
	};

	inline EventPQ* event_pq;

	inline void Thread(HANDLE iocp, int id)
	{
		constexpr auto kSleep = std::chrono::milliseconds{ 200000 };
		while (true) {
			Event event;
			auto current_tp = Clock::now();
			if (true == event_pq[id].try_pop(event)) {
				if (event.tp > current_tp) {
					event_pq[id].push(event);
					std::this_thread::sleep_for(kSleep);
					continue;
				}
				switch (event.type) {
				case EventType::kNone:
					exit(1);
				/*case EventType::kRandomMove:
				{
					auto ox = free_list<OverEx>.Get(Operation::kNPCMove);
					PostQueuedCompletionStatus(iocp, 1, event.id, &ox->over);
					break;
				}*/
				}
				continue;
			}
			std::this_thread::sleep_for(kSleep);
		}
	}
}