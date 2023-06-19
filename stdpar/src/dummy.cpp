#include <vector>
// #include <algorithm>
// #include <execution>
#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>
#include <iostream>

int main() {
    sycl::usm_allocator<float, sycl::usm::alloc::shared> alloc(oneapi::dpl::execution::dpcpp_default.queue());
    std::vector<float, sycl::usm_allocator<float, sycl::usm::alloc::shared>> v(alloc);

    // std::allocator<float> alloc;
    // std::vector<float> v(alloc);

    v.resize(8000000);

    auto start = std::chrono::high_resolution_clock::now();

    oneapi::dpl::fill(oneapi::dpl::execution::dpcpp_default, v.data(), v.data() + v.size(), 1);
    // oneapi::dpl::fill(oneapi::dpl::execution::dpcpp_default, v.begin(), v.end(), 1);

    auto finish = std::chrono::high_resolution_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << std::endl;
}
