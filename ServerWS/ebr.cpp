#include "ebr.h"

lf::EBR::EBR(int thread_num) : reservations{ new Epoch[thread_num] }, epoch{}
{
	for (int i = 0; i < thread_num; ++i) {
		reservations[i] = std::numeric_limits<Epoch>::max();
	}
}

lf::EBR::~EBR()
{
	delete[] reservations;
}

void lf::EBR::Retire(lf::Node* ptr)
{
	ptr->retire_epoch = epoch.load(std::memory_order_relaxed);
	GetRetired().push(ptr);
	if (GetRetired().size() >= GetCapacity()) {
		Clear();
	}
}

void lf::EBR::StartOp()
{
	reservations[thread::GetID()] = epoch.fetch_add(1, std::memory_order_relaxed);
}

void lf::EBR::EndOp()
{
	reservations[thread::GetID()] = std::numeric_limits<Epoch>::max();
}

lf::EBR::RetiredNodeQueue& lf::EBR::GetRetired()
{
	static thread_local RetiredNodeQueue retired;
	return retired;
}

lf::EBR::Epoch lf::EBR::GetCapacity() const
{
	return (Epoch)(3 * thread::GetNumWorker() * 2 * 10);
}

lf::EBR::Epoch lf::EBR::GetMinReservation()
{
	Epoch min_re = std::numeric_limits<Epoch>::max();
	for (int i = 0; i < 32/*reservations.size()*/; ++i) {
		min_re = std::min(min_re, (Epoch)reservations[i]);
	}
	return min_re;
}

void lf::EBR::Clear()
{
	Epoch max_safe_epoch = GetMinReservation();

	while (false == GetRetired().empty()) {
		auto f = GetRetired().front();
		if (f->retire_epoch >= max_safe_epoch)
			break;
		GetRetired().pop();

		delete f;
	}
}