#include <vector>
#include <print>
#include "world_map.h"

WorldMap world_map;

WorldMap::WorldMap() noexcept
{
	std::vector<Region::ShapeFunc> shapes;
	shapes.reserve(12);

	shapes.emplace_back([](const Position& pos) {
		return IsInRectGrid(pos, Position{ 2.0f, 0.5f }, 4.0f, 1.0f);
		}); // 0
	shapes.emplace_back([](const Position& pos) {
		Position center{ 4.0f, 2.0f };
		return IsInRectGrid(pos, Position{ 5.0f, 1.0f }, 2.0f, 2.0f)
			and IsInCircleGrid(pos, center, 2.0f)
			and not IsInCircleGrid(pos, center, 1.0f);
		}); // 1
	shapes.emplace_back([](const Position& pos) {
		Position center{ 7.0f, 2.0f };
		return IsInRectGrid(pos, Position{ 6.0f, 3.0f }, 2.0f, 2.0f)
			and IsInCircleGrid(pos, center, 2.0f)
			and not IsInCircleGrid(pos, center, 1.0f);
		}); // 2
	shapes.emplace_back([](const Position& pos) {
		Position center{ 7.0f, 2.0f };
		return IsInRectGrid(pos, Position{ 8.0f, 3.0f }, 2.0f, 2.0f)
			and IsInCircleGrid(pos, center, 2.0f)
			and not IsInCircleGrid(pos, center, 1.0f);
		}); // 3
	shapes.emplace_back([](const Position& pos) {
		return IsInRectGrid(pos, Position{ 8.5f, 1.5f }, 1.0f, 1.0f);
		}); // 4
	shapes.emplace_back([](const Position& pos) {
		Position center{ 10.0f, 1.0f };
		return IsInRectGrid(pos, Position{ 9.0f, 0.0f }, 2.0f, 2.0f)
			and IsInCircleGrid(pos, center, 2.0f)
			and not IsInCircleGrid(pos, center, 1.0f);
		}); // 5
	shapes.emplace_back([](const Position& pos) {
		return IsInRectGrid(pos, Position{ 11.0f, -0.5f }, 2.0f, 1.0f);
		}); // 6
	shapes.emplace_back([](const Position& pos) {
		Position center{ 13.0f, -2.0f };
		return IsInRectGrid(pos, Position{ 13.0f, -1.0f }, 2.0f, 2.0f)
			and IsInCircleGrid(pos, center, 2.0f)
			and not IsInCircleGrid(pos, center, 1.0f);
		}); // 7
	shapes.emplace_back([](const Position& pos) {
		return IsInRectGrid(pos, Position{ 13.5f, -3.0f }, 1.0f, 2.0f);
		}); // 8
	shapes.emplace_back([](const Position& pos) {
		Position center{ 13.0f, -4.0f };
		return IsInRectGrid(pos, Position{ 13.0f, -5.0f }, 2.0f, 2.0f)
			and IsInCircleGrid(pos, center, 2.0f)
			and not IsInCircleGrid(pos, center, 1.0f);
		}); // 9
	shapes.emplace_back([](const Position& pos) {
		return IsInRectGrid(pos, Position{ 10.5f, -5.5f }, 3.0f, 1.0f);
		}); // 10
	shapes.emplace_back([](const Position& pos) {
		return IsInRectGrid(pos, Position{ 5.5f, -5.5f }, 7.0f, 1.0f)
			or IsInRectGrid(pos, Position{ 5.5f, -5.5f }, 1.0f, 7.0f)
			or IsInCircleGrid(pos, Position{ 6.0f, -5.0f }, 3.0f)
			or IsInCircleGrid(pos, Position{ 5.0f, -5.0f }, 3.0f)
			or IsInCircleGrid(pos, Position{ 5.0f, -6.0f }, 3.0f)
			or IsInCircleGrid(pos, Position{ 6.0f, -6.0f }, 3.0f);
		}); // 11

	for (const auto& shape : shapes) {
		regions_.emplace_back(shape);
	}

	regions_[0].AddAdjRegion(&regions_[1]);

	for (int i = 1; i <= 10; ++i) {
		regions_[i].AddAdjRegion(&regions_[i - 1]);
		regions_[i].AddAdjRegion(&regions_[i + 1]);
	}

	regions_[11].AddAdjRegion(&regions_[10]);
}