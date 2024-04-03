//---------------------------------------------------
// 
// over_ex.h - OVERLAPPED Ȯ�� Ŭ���� ����
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
	void Reset(Operation reset_op) noexcept {
		op = reset_op;
		memset(&over, 0, sizeof(over));
	}
	OVERLAPPED over;
	Operation op;
};