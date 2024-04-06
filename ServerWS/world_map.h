#pragma once
#include "region.h"

class WorldMap {
public:
	WorldMap() noexcept;
private:
	std::vector<Region> regions_;
};