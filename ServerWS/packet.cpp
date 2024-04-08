#include <print>
#include "packet.h"
#include "free_list.h"
#include "debug.h"

namespace packet {
	void Collect(void* p) noexcept
	{
		auto packet = reinterpret_cast<Base*>(p);
		switch (packet->type)
		{
		case Type::kTest: {
			using Packet = Test;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCNewEntity: {
			using Packet = SCNewEntity;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCPosition: {
			using Packet = SCPosition;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCRemoveEntity: {
			using Packet = SCRemoveEntity;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCResult: {
			using Packet = SCRemoveEntity;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		case Type::kSCCheckConnection: {
			using Packet = SCCheckConnection;
			free_list<Packet>.Collect(reinterpret_cast<Packet*>(p));
			break;
		}
		default: {
			if (debug::DisplaysMSG()) {
				std::print("[Error] Unknown Packet: {}\n", static_cast<int>(packet->type));
				exit(1);
			}
			break;
		}
		}
	}
}