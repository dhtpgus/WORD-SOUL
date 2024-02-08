#pragma once
#include <random>
#include <memory>

class RandomNumberGenerator {
public:
	RandomNumberGenerator() = default;

	template<class T>
	T Rand(T min, T max) {
		double rand_num = static_cast<double>((*uid_)(*dre_));
		rand_num /= std::numeric_limits<long long>::max();

		return min + (max - min) * static_cast<T>(rand_num);
	}
private:
	std::unique_ptr<std::random_device> rd_;
	std::unique_ptr<std::default_random_engine> dre_;
	std::unique_ptr<std::uniform_int_distribution<long long>> uid_;
};

extern RandomNumberGenerator rng;