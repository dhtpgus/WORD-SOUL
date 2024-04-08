//---------------------------------------------------
// 
// over_ex.h - OVERLAPPED 확장 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

enum class Operation {
	kNone, kSend, kRecv, kAccept
};

struct OverEx {
	OverEx() noexcept : op{ Operation::kNone } {
		memset(&over, 0, sizeof(over));
	}
	OverEx(Operation op) noexcept : op{ op } {
		memset(&over, 0, sizeof(over));
	}
	void Reset(Operation rs_op) noexcept {
		op = rs_op;
		memset(&over, 0, sizeof(over));
	}
	OVERLAPPED over;
	Operation op;
};