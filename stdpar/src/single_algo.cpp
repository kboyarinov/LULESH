#include <random>
#include <vector>
#include <chrono>
#include <iostream>

#include <algorithm>
#include <execution>
#include <sycl/sycl.hpp>

int main() {
    std::mt19937 random_generator(42);
    std::size_t n = 50000000;
    std::size_t scalar = 42;

    using T = float;

    T* data_from1 = reinterpret_cast<T*>(sycl::malloc_shared(n * sizeof(T), oneapi::dpl::execution::dpcpp_default.queue()));
    T* data_from2 = reinterpret_cast<T*>(sycl::malloc_shared(n * sizeof(T), oneapi::dpl::execution::dpcpp_default.queue()));
    T* data_to = reinterpret_cast<T*>(sycl::malloc_shared(n * sizeof(T), oneapi::dpl::execution::dpcpp_default.queue()));

    for (std::size_t i = 0; i < n; ++i) {
        ::new(data_from1 + i) T(random_generator());
        ::new(data_from2 + i) T(random_generator());
        ::new(data_to + i) T(0);
    }

    std::vector<double> times;
    std::size_t n_times = 11;
    times.reserve(n_times);

    for (std::size_t i = 0; i < n_times; ++i) {
        std::vector<T> tmp(n);

        auto start = std::chrono::high_resolution_clock::now();

        std::fill(std::execution::par_unseq, data_from1, data_from1 + n, 1);

        auto finish = std::chrono::high_resolution_clock::now();
        times.emplace_back(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
    }

    std::cout << "Run completed" << std::endl;
    std::sort(times.begin(), times.end());
    std::cout << "Elapsed time: " << times[n_times / 2] << " ms" << std::endl;

    for (std::size_t i = 0; i < n; ++i) {
        (data_from1 + i)->~T();
        (data_from2 + i)->~T();
        (data_to + i)->~T();
    }

    sycl::free(data_from1, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(data_from2, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(data_to, oneapi::dpl::execution::dpcpp_default.queue());
}
