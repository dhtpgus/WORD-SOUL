//---------------------------------------------------
// 
// debug.h - ����� ���� �Լ� ����
// 
//---------------------------------------------------

#pragma once
#define IS_RELEASE_MODE 0

namespace debug {
#if IS_RELEASE_MODE
	inline constexpr bool DisplaysMSG() noexcept {
		return false;
	};
#else
	bool DisplaysMSG() noexcept;
#endif
}