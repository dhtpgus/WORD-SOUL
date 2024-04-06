#pragma once
#include <vector>
#include <functional>
#include "coord.h"

class Region {
public:
	using Shape = std::function<bool(const Position&)>;
	Region(const Shape& shape) noexcept : shape_{ shape } {}
	void AddAdjRegion(Region* r) noexcept {
		adjs_.push_back(r);
	}
	bool IsInRegion(const Position& pos) const {
		return shape_(pos);
	}
private:
	Shape shape_;
	std::vector<Region*> adjs_;
};

