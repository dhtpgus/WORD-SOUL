#include "random_number_generator.h"

RandomNumberGenerator rng;

RandomNumberGenerator::RandomNumberGenerator() noexcept
{
	rd_ = std::make_unique<std::random_device>();
	dre_ = std::make_unique<std::default_random_engine>((*rd_)());
	uid_ = std::make_unique<std::uniform_int_distribution<long long>>();
}