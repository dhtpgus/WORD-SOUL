#pragma once
#include <queue>
#include <vector>
#include <atomic>
#include "thread.h"
#include "lf_node.h"

namespace ebr {

	class EBR {
	public:
		EBR() : reservations(thread::GetNumWorker(), std::numeric_limits<Epoch>::max()),
			epoch{} {}
	private:
		using Epoch = unsigned long long;
		using RetiredNodeQueue = std::queue<lf::Node*>;

		RetiredNodeQueue& GetRetired() {
			static thread_local RetiredNodeQueue retired;
			return retired;
		}

		Epoch GetCapacity() const {
			return (Epoch)(3 * thread::GetNumWorker() * 2 * 10);
		}

		Epoch GetMinReservation() {
			Epoch min_re = std::numeric_limits<Epoch>::max();
			for (int i = 0; i < reservations.size(); ++i) {
				min_re = std::min(min_re, (Epoch)reservations[i]);
			}
			return min_re;
		}

		void Clear() {
			Epoch max_safe_epoch = GetMinReservation();

			while (false == GetRetired().empty()) {
				auto f = GetRetired().front();
				if (f->retire_epoch >= max_safe_epoch)
					break;
				GetRetired().pop();

				delete f;
			}
		}

		void Retire(lf::Node* ptr) {
			ptr->retire_epoch = epoch.load(std::memory_order_relaxed);
			GetRetired().push(ptr);
			if (GetRetired().size() >= GetCapacity()) {
				Clear();
			}
		}

		void StartOp() {
			reservations[thread::GetID()] = epoch.fetch_add(1, std::memory_order_relaxed);
		}

		void EndOp() {
			reservations[thread::GetID()] = std::numeric_limits<Epoch>::max();
		}

		std::vector<Epoch> reservations;
		std::atomic<Epoch> epoch;
	};
}