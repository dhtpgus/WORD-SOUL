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
	kRecv, kSend, kAccept
};

struct OverEx {
public:
	OverEx() : buf{}, op{ Operation::kRecv } {
		wsabuf.len = sizeof(buf);
		wsabuf.buf = buf;
		memset(&over, 0, sizeof(over));
	}
	OverEx(char* packet) {
		wsabuf.len = packet[0];
		wsabuf.buf = buf;
		memset(&over, 0, sizeof(over));
		op = Operation::kSend;
		memcpy(buf, packet, packet[0]);
	}
	OVERLAPPED over;
	WSABUF wsabuf;
	char buf[1024];
	Operation op;
};