#include <oneapi/dpl/iterator>

#include <new>
#include <random>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <execution>
#include <sycl/sycl.hpp>

template <typename Body>
void measure(const Body& body) {
    std::size_t n_times = 11;
    std::vector<double> times;
    times.reserve(n_times);

    for (std::size_t i = 0; i < n_times; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        body();
        auto finish = std::chrono::high_resolution_clock::now();
        times.emplace_back(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
    }
    std::sort(times.begin(), times.end());
    std::cout << "Elapsed time (median): " << times[n_times / 2] << " mcs" << std::endl;
}

int main() {
    std::mt19937 generator;
    std::size_t n = 30000000;

#ifdef USE_SYCL_USM
    float* a = sycl::malloc_shared<float>(n, oneapi::dpl::execution::dpcpp_default.queue());
    float* b = sycl::malloc_shared<float>(n, oneapi::dpl::execution::dpcpp_default.queue());
    float* c = sycl::malloc_shared<float>(n, oneapi::dpl::execution::dpcpp_default.queue());
#else
    float* a = reinterpret_cast<float*>(malloc(n * sizeof(float)));
    float* b = reinterpret_cast<float*>(malloc(n * sizeof(float)));
    float* c = reinterpret_cast<float*>(malloc(n * sizeof(float)));
#endif
    float scalar = 42.f;

    auto stream_triad_body = [=, &generator]() {
        std::generate(std::execution::par_unseq, b, b + n, generator);
        std::generate(std::execution::par_unseq, c, c + n, generator);
        std::fill(std::execution::par_unseq, a, a + n, 0.f);

        std::transform(std::execution::par_unseq, oneapi::dpl::counting_iterator<std::size_t>(0), oneapi::dpl::counting_iterator<std::size_t>(n), a,
                       [=](std::size_t index) {
                           return b[index] + c[index] * scalar;
                       });
    };

    measure(stream_triad_body);

#ifdef USE_SYCL_USM
    sycl::free(c, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(b, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(a, oneapi::dpl::execution::dpcpp_default.queue());
#else
    free(c);
    free(b);
    free(a);
#endif
}