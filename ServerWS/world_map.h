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
		return -1;
	}
	bool IsInSameRegion(const Position& p1, const Position& p2) const noexcept {
		auto r = FindRegion(p1);
		if (r == -1) {
			return false;
		}
		return regions_[r].IsInRegion(p2);
	}
private:
	std::vector<Region> regions_;
};

extern WorldMap world_map;