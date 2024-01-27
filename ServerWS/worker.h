//---------------------------------------------------
// 
// worker.h - 작업자 스레드 함수 정의
// 
//---------------------------------------------------

#pragma once
#include <iostream>
#include <unordered_set>
#include "server_socket.h"
#include "thread.h"
#include "relaxed_queue.h"

namespace thread {
	void Worker(HANDLE iocp, int thread_id, lf::RelaxedQueue* rq)
	{
		DWORD transferred;
		SOCKET client_sock;
		client::Socket* client_ptr;
		int retval;

		while (true) {

			int t = 2;

			rq->Push(&t);

			int* v = (int*)rq->Pop();

			continue;

			retval = GetQueuedCompletionStatus(iocp, &transferred, &client_sock,
				(LPOVERLAPPED*)&client_ptr, 1);

			if (retval == 0) {
				continue;
			}

			else if (retval != 0 and transferred != 0) {
				std::cout << std::format("{}: {}\n", client_sock, client_ptr->GetBuffer());
				client_ptr->StartAsyncIO();
			}
		}
	}
}