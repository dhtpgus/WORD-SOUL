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

		/*void Reset(int rs_depth, float rs_cur_x, float rs_cur_y, float rs_dst_x, float rs_dst_y) {
			depth_ = rs_depth;
			cur_x_ = rs_cur_x;
			cur_y_ = rs_cur_y;
			dst_x_ = rs_dst_x;
			dst_y_ = rs_dst_y;
		}*/

		auto Expand(StateVector& next) const noexcept {
			const float kPi{ acosf(-1) };

			for (int i = 0; i < kNumExpand; ++i) {
				auto next_x = cur_.x + cosf(kPi * 2 * i / kNumExpand) * distance_;
				auto next_y = cur_.y + sinf(kPi * 2 * i / kNumExpand) * distance_;
				
				if (WorldMap::kOutOfBounds != world_map.FindRegion(Position{ next_x, next_y, 0.0f })) {
					next.emplace_back(depth_ + 1, dir_, next_x, next_y, dst_.x, dst_.y, distance_);
					if (next.back().dir_ == kDirUndefined) {
						next.back().dir_ = kPi * 2 * i / kNumExpand;
					}
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

		static constexpr size_t kNumExpand{ 40 };
		static constexpr float kDirUndefined{ -1.0f };
	private:
		int depth_;
		float dir_;
		float distance_;
		Position cur_;
		Position dst_;
	};

	inline Position GetNextPosition(const Position& cur, const Position& trg, float time, float speed) noexcept
	{
		constexpr auto kMaxTries{ 40 };

		StatePQ open_queue;
		open_queue.emplace(0, State::kDirUndefined, cur.x, cur.y, trg.x, trg.y, time * speed);
		StateVector next_states;
		next_states.reserve(State::kNumExpand);

		for (int i = 0; i < kMaxTries; ++i) {
			next_states.clear();

			if (open_queue.empty()) {
				return cur;
			}
			const State& state = open_queue.top();
			open_queue.pop();

			if (true == state.Check()) {
				return Position{ cur.x + cosf(state.GetDir()) * time * speed,
					cur.y + sinf(state.GetDir()) * time * speed, cur.z };
			}

			state.Expand(next_states);

			for (const auto& state : next_states) {
				open_queue.push(state);
			}
		}
		auto dir = open_queue.top().GetDir();

		return Position{ cur.x + cosf(dir) * time * speed,
			cur.y + sinf(dir) * time * speed, cur.z };
	}
}