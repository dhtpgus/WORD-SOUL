#pragma once
#include "region.h"

class WorldMap {
public:
	WorldMap() noexcept;
	~WorldMap() noexcept = default;
	WorldMap(const WorldMap&) = delete;
	WorldMap(WorldMap&&) = delete;
	WorldMap& operator=(const WorldMap&) = delete;
	WorldMap& operator=(WorldMap&&) = delete;

	int FindRegion(const Position& p) const noexcept {
		for (int i = 0; i < regions_.size(); ++i) {
			if (regions_[i].IsInRegion(p)) {
				return i;
			}
		}
		return kOutOfBounds;
	}
	bool IsInSameRegion(const Position& p1, const Position& p2) const noexcept {
		auto r = FindRegion(p1);
		if (r == kOutOfBounds) {
			return false;
		}
		return regions_[r].IsInRegion(p2);
	}
	Region& GetRegion(int i) {
		return regions_[i];
	}

	static constexpr auto kOutOfBounds{ -1 };
	static constexpr auto kNumRegions{ 12 };
private:
	std::vector<Region> regions_;
};

inline WorldMap world_map;

inline bool IsValidRegion(int r_id)
{
	return r_id >= 0 and r_id < WorldMap::kNumRegions;
}