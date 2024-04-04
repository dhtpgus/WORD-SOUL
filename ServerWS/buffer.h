#pragma once
#include <array>

inline constexpr size_t kBufferSize{ 256 };

using Buffer = std::array<char, kBufferSize>;

class BufferRecv {
public:
	BufferRecv() noexcept = default;
	auto GetData() const noexcept {
		return &buf_[cursor_];
	}
	void SaveRemains(int size_remains) noexcept {
		memcpy(&buf_[kStartPoint - size_remains], &buf_[cursor_], size_remains);
		cursor_ = kStartPoint - size_remains;
	}
	void MoveCursor(int diff) noexcept {
		cursor_ += diff;
	}
	void ResetCursor() noexcept {
		cursor_ = kStartPoint;
	}
	std::string GetBinary(int size) const noexcept
	{
		std::string data;
		for (int i = 0; i < size; ++i) {
			data += std::format("{:02X} ", static_cast<unsigned int>(
				static_cast<unsigned char>(*(GetData() + i))));
		}
		return std::format("send {} bytes: {}", size, data);
	}
private:
	static constexpr int kStartPoint{ kBufferSize };
	std::array<char, kBufferSize * 2> buf_{};
	int cursor_{ kStartPoint };
};