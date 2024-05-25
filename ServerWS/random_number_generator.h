//----------------------------------------------------------------
// 
// random_number_generator.h - RandomNumberGenerator 클래스 정의
// 
//----------------------------------------------------------------

#pragma once
#include <random>
#include <memory>
#undef max
#undef min

class RandomNumberGenerator {
public:
	RandomNumberGenerator() noexcept;

	template<class T> requires std::integral<T>
	T Rand(T min, T max) noexcept {
		auto rand_num = ((*uid_)(*dre_));

		return min + rand_num % (max - min + 1);
	}

	template<class T> requires std::floating_point<T>
	T Rand(T min, T max) noexcept {
		double rand_num = static_cast<double>((*uid_)(*dre_));
		rand_num /= std::numeric_limits<long long>::max();

		return min + static_cast<T>((max - min) * (rand_num));
	}
private:
	std::unique_ptr<std::random_device> rd_;
	std::unique_ptr<std::default_random_engine> dre_;
	std::unique_ptr<std::uniform_int_distribution<long long>> uid_;
};

inline thread_local RandomNumberGenerator rng;