#pragma once
#include <cmath>

struct Position {
	Position() noexcept = default;
	Position(float x, float y, float z) noexcept : x{ x }, y{ y }, z{ z } {}
	Position(float grid_x, float grid_y) noexcept
		: x{ kPivotX + kUnitLength * grid_x }, y{ kPivotY + kUnitLength * grid_y }, z{ 0.0f } {}
	
	static constexpr float kPivotX = 0.0f;
	static constexpr float kPivotY = 0.0f;
	static constexpr float kUnitLength = 1000.0f;
	float x{}, y{}, z{};
};

struct Vector {
	Vector(const Position& start, const Position& end) noexcept 
		: x{ end.x - start.x }, y{ end.z - start.z }, z{ end.z - start.z } {}
	void Normalize() noexcept {
		float scalar{ sqrtf(x * x + y * y + z * z) };
		x /= scalar;
		y /= scalar;
		z /= scalar;
	}
	Vector& operator*=(int scalar) noexcept {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	float x, y, z;
};

inline Position& operator+=(Position& p, const Vector& v) noexcept
{
	p.x += v.x;
	p.y += v.y;
	p.z += v.z;
}

inline float GetDistanceSq(const Position& a, const Position& b)
{
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
}

inline float GetDistance2DSq(const Position& a, const Position& b)
{
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

inline bool IsInRectGrid(const Position& p, const Position& center, float w, float h)
{
	w *= Position::kUnitLength;
	h *= Position::kUnitLength;
	return center.x - w / 2 < p.x and p.x < center.x + w / 2
		and center.y - h / 2 < p.y and p.y < center.y + h / 2;
}

inline bool IsInCircleGrid(const Position& p, Position& center, float r)
{
	r *= Position::kUnitLength;
	float tmp_z = center.z;
	center.z = p.z;
	bool ret = GetDistanceSq(p, center) <= r * r;
	center.z = tmp_z;
	return ret;
}

inline bool IsInCircleGrid(const Position& p, Position&& center, float r)
{
	r *= Position::kUnitLength;
	float tmp_z = center.z;
	center.z = p.z;
	bool ret = GetDistanceSq(p, center) <= r * r;
	center.z = tmp_z;
	return ret;
}