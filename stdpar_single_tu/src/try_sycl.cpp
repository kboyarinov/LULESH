#include <sycl/sycl.hpp>
#include <iostream>
#include <cassert>
#include <algorithm>

class Kernel;

int main() {
    auto available_platforms = sycl::platform::get_platforms();

    auto& lz_platform = *std::find_if(available_platforms.begin(), available_platforms.end(),
                                      [](auto& platform) {
                                          return platform.template get_info<sycl::info::platform::name>().find("Level-Zero") != std::string::npos;
                                      });

    auto available_devices = lz_platform.get_devices();
    auto& gpu_device = available_devices.front();

    assert(gpu_device.get_info<sycl::info::device::device_type>() == sycl::info::device_type::gpu);

    sycl::queue queue1(gpu_device);
    sycl::queue queue2(gpu_device);

    auto device1 = queue1.get_device();
    auto device2 = queue2.get_device();

    auto context1 = queue1.get_context();
    auto context2 = queue2.get_context();

    std::cout << std::boolalpha << "Contexts equal? " << (context1 == context2) << std::endl;

    float* object = sycl::malloc_shared<float>(1, device1, context1);

    auto event = queue2.single_task([object] { *object = 42; });
    event.wait();

    std::cout << "object equal to " << *object << std::endl;

    sycl::free(object, context1);
}
