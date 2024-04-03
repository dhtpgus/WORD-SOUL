#pragma once
#include <array>

inline constexpr size_t kBufferSize{ 256 };

using Buffer = std::array<char, kBufferSize>;