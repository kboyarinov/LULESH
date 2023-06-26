
#include <random>
#include <vector>
#include <chrono>
#include <iostream>

#include <algorithm>
#include <execution>
#include <sycl/sycl.hpp>
#include <cassert>

constexpr std::size_t n = 27000000;
using counting_iterator = oneapi::dpl::counting_iterator<std::size_t>;

int main() {
    float* memory = new float[n];
    std::fill(std::execution::par_unseq, memory, memory + n, 42.f);

    for (std::size_t i = 0; i < n; ++i) {
        assert(memory[i] == 42.f);
    }

    float* memory2 = new float[n];
    float* memory3 = new float[n];

    std::for_each(std::execution::par_unseq, counting_iterator(0), counting_iterator(n),
                  [](std::size_t index) {
                    memory2[index] = memory[index] * 42;
                    memory3[index] = memory[index] * memory[index];
                  });

    for (std::size_t i = 0; i < n; ++i) {
        assert(memory[i] == 42.f);
        assert(memory2[index] == memory[index] * 42);
        assert(memory3[index] == memory[index] * memory[index]);
    }

    bool result = std::any_of(std::execution::par_unseq, memory2, memory2 + n, [](float f) { return f == 837.f; });
    assert(result == false);

    delete[] memory3;
    delete[] memory2;
    delete[] memory;
}
