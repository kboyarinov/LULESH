#include <oneapi/dpl/iterator>
#include <oneapi/dpl/execution>

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

    float scalar = 42.f;

    auto stream_triad_body = [=, &generator]() {

#ifdef USE_SYCL_USM
    float* a = sycl::malloc_shared<float>(n, oneapi::dpl::execution::dpcpp_default.queue());
    float* b = sycl::malloc_shared<float>(n, oneapi::dpl::execution::dpcpp_default.queue());
    float* c = sycl::malloc_shared<float>(n, oneapi::dpl::execution::dpcpp_default.queue());
#else
    float* a = reinterpret_cast<float*>(malloc(n * sizeof(float)));
    float* b = reinterpret_cast<float*>(malloc(n * sizeof(float)));
    float* c = reinterpret_cast<float*>(malloc(n * sizeof(float)));
#endif

#ifdef MEASURE_EACH
        auto start = std::chrono::high_resolution_clock::now();
#endif
        std::fill(std::execution::par_unseq, a, a + n, 0.f);
        std::fill(std::execution::par_unseq, b, b + n, 42.f);
        std::fill(std::execution::par_unseq, c, c + n, 43.f);
#ifdef MEASURE_EACH
        auto finish = std::chrono::high_resolution_clock::now();
        std::cout << "Fill block " << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << std::endl;
        start = std::chrono::high_resolution_clock::now();
#endif


        std::transform(std::execution::par_unseq, oneapi::dpl::counting_iterator<std::size_t>(0), oneapi::dpl::counting_iterator<std::size_t>(n), a,
                       [=](std::size_t index) {
                           return b[index] + c[index] * scalar;
                       });

#ifdef MEASURE_EACH
        finish = std::chrono::high_resolution_clock::now();
        std::cout << "Transform " << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << std::endl;
        start = std::chrono::high_resolution_clock::now();
#endif

        if (!std::all_of(std::execution::par_unseq, oneapi::dpl::counting_iterator<std::size_t>(0), oneapi::dpl::counting_iterator<std::size_t>(n),
                         [=](std::size_t index) {
                            return a[index] == b[index] + c[index] * scalar;
                         }))
            throw "Incorrect result";
#ifdef MEASURE_EACH
        finish = std::chrono::high_resolution_clock::now();
        std::cout << "all_of " << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() << std::endl;
#endif

#ifdef USE_SYCL_USM
    sycl::free(c, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(b, oneapi::dpl::execution::dpcpp_default.queue());
    sycl::free(a, oneapi::dpl::execution::dpcpp_default.queue());
#else
    free(c);
    free(b);
    free(a);
#endif

    };

    measure(stream_triad_body);
}