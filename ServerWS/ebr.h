//---------------------------------------------------
// 
// ebr.h - ebr 메모리 관리자 클래스 정의
// 
//---------------------------------------------------

#pragma once
#include <queue>
#include <vector>
#include <atomic>
#include <iostream>
#include "thread.h"
#include "lf_node.h"

namespace lf {
	class EBR {
	public:
		EBR() = delete;
		EBR(int thread_num);
		~EBR();
		EBR(const EBR&) = delete;
		EBR(EBR&&) = delete;
		EBR& operator=(const EBR&) = delete;
		EBR& operator=(EBR&&) = delete;
		void Retire(lf::Node* ptr);
		void StartOp();
		void EndOp();

	private:
		using Epoch = unsigned long long;
		using RetiredNodeQueue = std::queue<lf::Node*>;

		RetiredNodeQueue& GetRetired();
		Epoch GetCapacity() const;
		Epoch GetMinReservation() const;
		void Clear();

		Epoch* volatile reservations;
		std::atomic<Epoch> epoch;
	};
}