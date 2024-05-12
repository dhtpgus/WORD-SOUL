#pragma once
#include <vector>
#include <unordered_set>
#include <functional>
#include "coord.h"

class Region {
public:
	using ShapeFunc = std::function<bool(const Position&)>;
	Region(const ShapeFunc& shape, int id) noexcept : shape_funcs_{ shape }, id_{ id } {}
	void AddAdjRegion(Region* r) noexcept {
		adjs_.insert(r);
	}
	bool IsInRegion(const Position& pos) const {
		return shape_funcs_(pos);
	}
	auto& GetAdjRegions() const {
		return adjs_;
	}
	int GetID() const {
		return id_;
	}
private:
	ShapeFunc shape_funcs_;
	std::unordered_set<Region*> adjs_;
	int id_;
};

