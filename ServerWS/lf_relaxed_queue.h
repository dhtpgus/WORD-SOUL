//-----------------------------------------------------------------
// 
// lf_relaxed_queue.h - lockfree �ڷᱸ�� relaxed queue ����
// 
//-----------------------------------------------------------------

#pragma once
#include <atomic>
#include "free_list.h"
#include "thread.h"
#include "lf_node.h"
#include "ebr.h"

namespace lf {
	const inline Node::Value kRetryRequired = reinterpret_cast<Node::Value>(1);
	constexpr inline Node::Value kPopFailed = nullptr;

	class alignas(std::hardware_destructive_interference_size) LFQueue {
	public:
		LFQueue() noexcept {
			head = tail = free_list<Node>.Get(Node::Value{}, Node::Level{});
		}
		void Push(Node* e, const LFQueue& main_queue) noexcept {
			while (true) {
				Node* last = tail;
				Node* next = last->next;
				if (last != tail) {
					continue;
				}

				e->level = main_queue.GetTailLevel();

				if (nullptr == next) {
					if (true == CAS(&last->next, nullptr, e)) {
						CAS(&tail, last, e);
						return;
					}
				}
				else {
					CAS(&tail, last, next);
				}
			}
		}

		bool TryPush(Node* e, Node::Level lv) noexcept {
			e->level = lv;

			Node* last = tail;
			Node* next = last->next;
			if (last != tail) {
				return false;
			}

			if (tail->level != lv - 1) {
				return false;
			}

			if (nullptr == next) {
				if (true == CAS(&last->next, nullptr, e)) {
					CAS(&tail, last, e);
					return true;
				}
				return false;
			}
			//CAS(&tail, last, next);
			return false;
		}

		Node::Value Pop(EBR<Node>& ebr, Node::Level level = 0) noexcept {
			while (true) {
				Node* first = head;
				Node* last = tail;
				Node* next = first->next;
				if (first != head) {
					continue;
				}
				if (nullptr == next) {
					return kPopFailed;
				}

				if (level != 0 and next->level != level) {
					return kRetryRequired;
				}

				if (first == last) {
					CAS(&tail, last, next);
					continue;
				}
				Node::Value value = next->v;
				if (false == CAS(&head, first, next)) {
					continue;
				}
				ebr.Retire(first);
				return value;
			}
		}

		Node::Level GetTailLevel() const noexcept {
			return tail->level;
		}

		Node::Level GetHeadLevel() const noexcept {
			Node* next = head->next;
			if (next == nullptr) {
				return 0;
			}
			return next->level;
		}

		bool IsEmpty() const noexcept {
			return nullptr == head->next;
		}

		template<class T>
		void Clear() noexcept {
			while (nullptr != head->next) {
				Node* t = head;
				head = head->next;
				delete t;
			}
			tail = head;
		}
	private:
		bool CAS(Node* volatile* next, Node* old_p, Node* new_p) noexcept {
			return std::atomic_compare_exchange_strong(
				reinterpret_cast<volatile std::atomic_llong*>(next),
				reinterpret_cast<long long*>(&old_p),
				reinterpret_cast<long long>(new_p));
		}
		Node* volatile head;
		Node* volatile tail;
	};

	template<class T>
	class RelaxedQueue {
	public:
		RelaxedQueue() = delete;
		RelaxedQueue(int num_thread) noexcept : num_thread_{ num_thread },
			queues_(num_thread + 1), ebr_{ num_thread } {}
		~RelaxedQueue() noexcept {
			for (auto& lfq : queues_) {
				lfq.Clear<T>();
			}
		}
		RelaxedQueue(const RelaxedQueue&) = delete;
		RelaxedQueue(RelaxedQueue&&) = delete;
		RelaxedQueue& operator=(const RelaxedQueue&) = delete;
		RelaxedQueue& operator=(RelaxedQueue&&) = delete;

		template<class Type, class... Value>
		void Emplace(Value&&... value) noexcept {
			//Node* e = new Node{ new Type{ value... }, 0 };
			Node* e = free_list<Node>.Get(free_list<Type>.Get(value...), Node::Level{});
			ebr_.StartOp();

			Node::Level top_level = queues_[num_thread_].GetTailLevel() + 1;

			if (false == queues_[num_thread_].TryPush(e, top_level)) {
				queues_[thread::ID()].Push(e, queues_[num_thread_]);
			}

			ebr_.EndOp();
		}

		T* Pop() noexcept {
			ebr_.StartOp();
			Node::Value ret{};
			Node::Level main_head_level{};
			Node::Level branch_head_level{};
			bool retry_flag{};

			while (true) {
				int id = thread::ID();
				for (int i = 0; i < num_thread_; ++i) {
					main_head_level = queues_[num_thread_].GetHeadLevel();
					if (main_head_level == 0) {
						ebr_.EndOp();
						return (T*)kPopFailed;
					}

					branch_head_level = queues_[id].GetHeadLevel();
					if (branch_head_level != 0 and branch_head_level <= main_head_level) {
						ret = queues_[id].Pop(ebr_);

						if (ret == kPopFailed) {
							continue;
						}
						if (ret == kRetryRequired) {
							if (branch_head_level != main_head_level) {
								retry_flag = true;
							}
							continue;
						}

						ebr_.EndOp();
						return (T*)ret;
					}
					id = (id + 1) % num_thread_;
				}

				if (true == retry_flag) {
					continue;
				}

				main_head_level = queues_[num_thread_].GetHeadLevel();
				if (main_head_level == 0) {
					ebr_.EndOp();
					return (T*)kPopFailed;
				}
				ret = queues_[num_thread_].Pop(ebr_, main_head_level);
				if (ret != kRetryRequired) {
					ebr_.EndOp();
					return (T*)ret;
				}
			}
		}
	private:
		int num_thread_;
		std::vector<LFQueue> queues_;
		EBR<Node> ebr_;
	};
}