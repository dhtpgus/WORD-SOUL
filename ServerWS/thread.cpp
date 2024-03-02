#include <atomic>
#include <print>
#include "thread.h"

int thread::GetNumWorker()
{
	return 2 * (int)std::thread::hardware_concurrency();
}

int thread::ID(int id_to_register)
{
	static thread_local int id = id_to_register;
	if (id == kUnregisteredID) {
		std::print("[����] ������ ���̵� ���� ����ؾ� �մϴ�.\n");
		system("pause");
		exit(1);
	}
	if (id_to_register != kUnregisteredID and id != id_to_register) {
		std::print("[����] ������ ���̵� �̹� ��ϵǾ����ϴ�.\n");
		system("pause");
		exit(1);
	}
	return id;
}