//---------------------------------------------------
// 
// debug.h - ����� ���� �Լ� ����
// 
//---------------------------------------------------

#pragma once
#define IS_RELEASE_MODE 0

namespace debug {
#if IS_RELEASE_MODE
	constexpr bool DisplaysMSG() noexcept;
#else
	bool DisplaysMSG() noexcept;
#endif
}