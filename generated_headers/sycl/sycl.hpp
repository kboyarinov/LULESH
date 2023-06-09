#ifndef TOP_LEVEL_STD
#define TOP_LEVEL_SYCL_SYCL_HPP
#define TOP_LEVEL_STD
#endif // TOP_LEVEL_STD

#include_next <sycl/sycl.hpp>

#ifdef TOP_LEVEL_SYCL_SYCL_HPP
#undef TOP_LEVEL_SYCL_SYCL_HPP
#undef TOP_LEVEL_STD

#endif // TOP_LEVEL_SYCL_SYCL_HPP
