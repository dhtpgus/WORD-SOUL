//---------------------------------------------------------
// 
// buffer.h - BufferRecv 클래스 정의 및 버퍼 관련 상수 정의
// 
//---------------------------------------------------------

#pragma once
#include <array>

inline constexpr size_t kBufferSize{ 512 };

class BufferRecv {
public:
	BufferRecv() noexcept = default;
	auto GetData() const noexcept {
		return &buf_[cursor_];
	}
	auto GetRecvPoint() noexcept {
		return &buf_[kRecvPoint];
	}
	void SaveRemains(int size_remains) noexcept {
		memcpy(&buf_[kRecvPoint - size_remains], &buf_[cursor_], size_remains);
		cursor_ = kRecvPoint - size_remains;
	}
	auto GetSizeRemains() const noexcept{
		return kRecvPoint - cursor_;
	}
	void MoveCursor(int diff) noexcept {
		cursor_ += diff;
	}
	void ResetCursor() noexcept {
		cursor_ = kRecvPoint;
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
	static constexpr int kRecvPoint{ kBufferSize };
	std::array<char, kBufferSize * 2> buf_{};
	int cursor_{ kRecvPoint };
};

using Buffer = std::array<char, kBufferSize>;