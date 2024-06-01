#pragma once
#include <cmath>
#include <chrono>
#include <print>

struct Position {
	Position() noexcept = default;
	Position(float x, float y, float z) noexcept : x{ x }, y{ y }, z{ z } {}
	Position(float grid_x, float grid_y) noexcept
		: x{ kPivotX + kUnitLength * grid_x }, y{ kPivotY + kUnitLength * grid_y }, z{ 0.0f } {}
	
	Position(const Position& other) noexcept {
		SetXYZ(other.x, other.y, other.z);
	}

	Position(Position&& other) noexcept {
		SetXYZ(other.x, other.y, other.z);
	}

	Position& operator=(const Position& other) {
		SetXYZ(other.x, other.y, other.z);
		return *this;
	}

	Position& operator=(Position&& other) noexcept {
		SetXYZ(other.x, other.y, other.z);
		return *this;
	}

	auto GetXY() const noexcept {
		float xy[2]{ 0.0f, 0.0f };
		*reinterpret_cast<long long*>(xy) = *reinterpret_cast<const long long*>(&x);
		return std::make_tuple(xy[0], xy[1]);
	}

	void SetXY(float nx, float ny) noexcept {
		float xy[2]{ nx, ny };
		*reinterpret_cast<long long*>(&x) = *reinterpret_cast<long long*>(xy);
	}
	void SetXYZ(float nx, float ny, float nz) {
		SetXY(nx, ny);
		z = nz;
	}

	float GetAngle(const Position& other) const {
		static const float kPi = acosf(-1);
		auto [sx, sy] = GetXY();
		auto [ox, oy] = other.GetXY();
		return atan2f(oy - sy, ox - sx);
	}

	void Print() const noexcept {
		std::print("(x, y, z) = ({}, {}, {})\n", x, y, z);
	}

	static constexpr float kPivotX = 0.0f;
	static constexpr float kPivotY = 0.0f;
	static constexpr float kUnitLength = 1000.0f;
	float x{ 1e9f }, y{ 1e9f }, z{ 1e9f };
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
	return center.x - w / 2 <= p.x and p.x <= center.x + w / 2
		and center.y - h / 2 <= p.y and p.y <= center.y + h / 2;
}

inline bool IsInCircleGrid(const Position& p, Position& center, float r)
{
	r *= Position::kUnitLength;
	bool ret = GetDistance2DSq(p, center) <= r * r;
	return ret;
}

inline bool IsInCircleGrid(const Position& p, Position&& center, float r)
{
	r *= Position::kUnitLength;
	bool ret = GetDistance2DSq(p, center) <= r * r;
	return ret;
}

inline float ConvertAngle(float angle) {
	static const auto kPi{ acosf(-1) };
	return fmod(angle + 2 * kPi, 2 * kPi);
}