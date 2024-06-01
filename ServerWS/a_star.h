#pragma once
#include <vector>
#include <queue>
#include <set>
#include <memory>
#include "world_map.h"

namespace a_star {
	class State;
	using StateVector = std::vector<State>;
	using StatePQ = std::priority_queue<State, StateVector>;

	class State {
	public:
		State(int depth, float dir, float cur_x, float cur_y, float dst_x, float dst_y, float distance) noexcept
			: depth_{ depth }, dir_{ dir }, distance_{ distance }, cur_ { cur_x, cur_y, 0.0f }, dst_{ dst_x, dst_y, 0.0f } {}

		auto Expand(StateVector& next, const std::unordered_map<int, Position>& others) const noexcept {
			const float kPi{ acosf(-1) };

			for (int i = 0; i < kNumExpand; ++i) {
				auto rad = kPi * 2 * i / kNumExpand;

				auto next_x = cur_.x + cosf(rad) * distance_;
				auto next_y = cur_.y + sinf(rad) * distance_;
				Position next_pos{ next_x, next_y, 0.0f };

				bool break_flag{};

				if (WorldMap::kOutOfBounds != world_map.FindRegion(next_pos)) {
					for (auto& [other_id, other_pos] : others) {
						if (GetDistance2DSq(other_pos, next_pos) < 80.0f) {
							break_flag = true;
							break;
						}
					}
					if (break_flag) {
						continue;
					}
					
					next.emplace_back(depth_ + 1, (dir_ == kDirUndefined) ? rad : dir_,
						next_x, next_y, dst_.x, dst_.y, distance_);
				}
			}
		}

		float H() const noexcept { return abs(cur_.x - dst_.x) + abs(cur_.y - dst_.y); }
		auto G() const noexcept { return distance_ * depth_; }
		auto F() const noexcept { return H() + G(); }

		bool operator<(const State& rhs) const noexcept {
			return F() > rhs.F();
		}

		auto& GetPosition() const noexcept {
			return cur_;
		}

		float GetDir() const noexcept {
			return dir_;
		}

		bool Check() const noexcept {
			return H() < 1.0f;
		}

		static constexpr size_t kNumExpand{ 12 };
		static constexpr float kDirUndefined{ -1.0f };
	private:
		int depth_;
		float dir_;
		float distance_;
		Position cur_;
		Position dst_;
	};

	inline Position GetNextPosition(const Position& cur, float& d, const Position& trg, float time, float speed, const std::unordered_map<int, Position>& others) noexcept
	{
		constexpr auto kMaxTries{ 25 };
		auto distance = time * speed;

		StatePQ open_queue;
		open_queue.emplace(0, State::kDirUndefined, cur.x, cur.y, trg.x, trg.y, distance);
		StateVector next_states;
		next_states.reserve(State::kNumExpand);

		for (int i = 0; i < kMaxTries; ++i) {
			next_states.clear();

			if (open_queue.empty()) {
				return cur;
			}
			State state = open_queue.top();
			open_queue.pop();

			if (true == state.Check()) {
				auto dir = state.GetDir();
				d = dir;
				return Position{ cur.x + cosf(dir) * distance, cur.y + sinf(dir) * distance, cur.z };
			}

			state.Expand(next_states, others);

			for (const auto& state : next_states) {
				open_queue.push(state);
			}
		}
		auto dir = open_queue.top().GetDir();

		d = dir;
		return Position{ cur.x + cosf(dir) * distance, cur.y + sinf(dir) * distance, cur.z };
	}
}