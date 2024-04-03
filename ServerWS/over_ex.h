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
	OverEx(Operation op) : buf{}, op{ op } {
		wsabuf.len = sizeof(buf);
		wsabuf.buf = buf;
		memset(&over, 0, sizeof(over));
	}
	OVERLAPPED over;
	WSABUF wsabuf;
	char buf[1024];
	Operation op;
};