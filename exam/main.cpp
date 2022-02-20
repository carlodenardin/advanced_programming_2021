#include <iostream>
#include <chrono>

#include "stack_pool.hpp"

/**
 * Result using emplace_back = 5.099 s 
 * Result using push_back = 4.194 s  
 *
 */

int main () {
    stack_pool<int> pool(1000);

	auto s1 = pool.new_stack();

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 100000000; ++i){
		s1 = pool.push(i, s1);
	}

	auto end = std::chrono::high_resolution_clock::now();

	std::cout << "Time in seconds: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << " ms"
        << std::endl;
}
