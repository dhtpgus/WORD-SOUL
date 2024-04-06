#pragma once
#include <vector>
#include <functional>
#include "coord.h"

class Region {
public:
	using ShapeFunc = std::function<bool(const Position&)>;
	Region(const ShapeFunc& shape) noexcept : shape_funcs_{ shape } {}
	void AddAdjRegion(Region* r) noexcept {
		adjs_.push_back(r);
	}
	bool IsInRegion(const Position& pos) const {
		return shape_funcs_(pos);
	}
private:
	ShapeFunc shape_funcs_;
	std::vector<Region*> adjs_;
};

