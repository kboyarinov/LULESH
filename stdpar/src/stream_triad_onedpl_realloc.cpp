#include <oneapi/dpl/iterator>
#include <oneapi/dpl/execution>
#include <oneapi/dpl/algorithm>

#include <vector>
#include <chrono>
#include <iostream>
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
    std::size_t n = 30000000;

    sycl::queue gpu_queue{sycl::gpu_selector_v};
    auto gpu_policy = oneapi::dpl::execution::make_device_policy(gpu_queue);

    float scalar = 42.f;

    auto stream_triad_body = [=, &gpu_policy]() {
        float* a = sycl::malloc_shared<float>(n, gpu_queue);
        float* b = sycl::malloc_shared<float>(n, gpu_queue);
        float* c = sycl::malloc_shared<float>(n, gpu_queue);

        oneapi::dpl::fill(gpu_policy, a, a + n, 0.f);
        oneapi::dpl::fill(gpu_policy, b, b + n, 42.f);
        oneapi::dpl::fill(gpu_policy, c, c + n, 43.f);

        oneapi::dpl::transform(gpu_policy, oneapi::dpl::counting_iterator<std::size_t>(0), oneapi::dpl::counting_iterator<std::size_t>(n), a,
                       [=](std::size_t index) {
                           return b[index] + c[index] * scalar;
                       });
        if (!oneapi::dpl::all_of(gpu_policy, oneapi::dpl::counting_iterator<std::size_t>(0), oneapi::dpl::counting_iterator<std::size_t>(n),
                         [=](std::size_t index) {
                            return a[index] == b[index] + c[index] * scalar;
                         }))
            throw "Incorrect result";

        sycl::free(c, gpu_queue);
        sycl::free(b, gpu_queue);
        sycl::free(a, gpu_queue);

    };

    measure(stream_triad_body);

}