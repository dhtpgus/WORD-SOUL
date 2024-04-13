#pragma once
#include <vector>
#include <queue>
#include <set>
#include <memory>
#include "world_map.h"

namespace a_star {
	class State;
	using StatePQ = std::set<State>;
	using StateVector = std::vector<State>;

	class State {
	public:
		State(int depth, float cur_x, float cur_y, float dst_x, float dst_y) noexcept
			: depth_{ depth }, cur_{ cur_x, cur_y, 0.0f }, dst_{ dst_x, dst_y, 0.0f } {}

		/*void Reset(int rs_depth, float rs_cur_x, float rs_cur_y, float rs_dst_x, float rs_dst_y) {
			depth_ = rs_depth;
			cur_x_ = rs_cur_x;
			cur_y_ = rs_cur_y;
			dst_x_ = rs_dst_x;
			dst_y_ = rs_dst_y;
		}*/

		auto Expand(StateVector& next, float time, float speed) const noexcept {
			const float kPi{ acosf(-1) };

			for (int i = 0; i < kNumExpand; ++i) {
				auto next_x = cur_.x + cosf(kPi * 2 * i / kNumExpand) * time * speed;
				auto next_y = cur_.y + sinf(kPi * 2 * i / kNumExpand) * time * speed;
				
				if (WorldMap::kOutOfBounds != world_map.FindRegion(Position{ next_x, next_y, 0.0f })) {
					next.emplace_back(depth_ + 1, next_x, next_y, dst_.x, dst_.y);
				}
			}
		}

		float H() const noexcept { return abs(cur_.x - dst_.x) + abs(cur_.y - dst_.y); }
		auto G() const noexcept { return 1000.0f * depth_; }
		auto F() const noexcept { return H() + G(); }

		bool operator==(const State& rhs) const noexcept {
			const float kEpsilon = 1e-5f;
			return abs(cur_.x - rhs.cur_.x) < kEpsilon
				and abs(cur_.y - rhs.cur_.y) < kEpsilon
				and dst_.x == rhs.dst_.x and dst_.x == rhs.dst_.y;
		}

		bool operator<(const State& rhs) const noexcept {
			if (*this == rhs) {
				return false;
			}
			if (depth_ > rhs.depth_) {
				return true;
			}
			else if (depth_ == rhs.depth_) {
				return F() < rhs.F();
			}
			return false;
		}
		auto& GetPosition() const noexcept {
			return cur_;
		}

		static constexpr size_t kNumExpand{ 40 };
	private:
		int depth_;
		Position cur_;
		Position dst_;
	};

	Position GetNextPosition(const Position& cur, const Position& trg, float time, float speed) noexcept
	{
		constexpr auto kMaxDepth{ 2 };

		StatePQ states;
		states.emplace(0, cur.x, cur.y, trg.x, trg.y);
		StateVector next_states;
		next_states.reserve(State::kNumExpand * State::kNumExpand);

		for (int d = 0; d < kMaxDepth; ++d) {
			next_states.clear();
			for (auto it = states.begin(); it != states.end(); ++it) {
				it->Expand(next_states, time, speed);
			}

			for (int i = 0; i < next_states.size(); ++i) {
				states.insert(next_states[i]);
			}
		}
		return states.begin()->GetPosition();
	}
}