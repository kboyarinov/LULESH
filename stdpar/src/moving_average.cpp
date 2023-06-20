#include <random>
#include <vector>
#include <chrono>

#include <algorithm>
#include <execution>
#include <sycl/sycl.hpp>

int main() {
    std::mt19937 random_generator(42);
    std::size_t n = 30000000;
    std::size_t average = 1000;

    using T = float;

    T* data_from = reinterpret_cast<T*>(sycl::malloc_shared(n * sizeof(T), oneapi::dpl::execution::dpcpp_default.queue()));
    T* data_to = reinterpret_cast<T*>(sycl::malloc_shared(n * sizeof(T), oneapi::dpl::execution::dpcpp_default.queue()));

    for (std::size_t i = 0; i < n; ++i) {
        ::new(data_from) T(random_generator());
        ::new(data_to) T(0);
    }

    std::vector<double> times;
    std::size_t n_times = 11;
    times.reserve(n_times);

    for (std::size_t i = 0; i < n; ++i) {
        std::vector<T> tmp(n);

        auto start = std::chrono::high_resolution_clock::now();

        std::exclusive_scan(std::execution::par_unseq, data_from, data_from + n, tmp.data(), T{});
        printf("check\n");
        std::transform(std::execution::par_unseq, tmp.data() + average, tmp.data() + n, tmp.data(), data_to,
                       [average](const T x, const T y) { return (x - y) / average; });
        printf("check2\n");

        auto finish = std::chrono::high_resolution_clock::now();
        times.emplace_back(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count());
    }

    for (std::size_t i = 0; i < n; ++i) {
        (data_from + i)->~T();
        (data_to + i)->~T();
    }

    sycl::free(data_from, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(data_to, oneapi::dpl::execution::dpcpp_default.queue());
}
