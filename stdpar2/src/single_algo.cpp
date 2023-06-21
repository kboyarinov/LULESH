#include <random>
#include <vector>
#include <chrono>
#include <iostream>

#include <algorithm>
#include <execution>
#include <sycl/sycl.hpp>

using T = float;
std::size_t n = 50000000;
std::mt19937 random_generator(42);

T* allocate(std::size_t count) {
    T* ptr =  reinterpret_cast<T*>(sycl::malloc_shared(count * sizeof(T), oneapi::dpl::execution::dpcpp_default.queue()));
    for (std::size_t i = 0; i < count; ++i) {
        ::new(ptr + i) T(random_generator());
    }
    return ptr;
}

void free(T* ptr, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        (ptr + i)->~T();
    }
    sycl::free(ptr, oneapi::dpl::execution::dpcpp_default.queue());
}

class Domain {
public:
    Domain()
        : m_e(allocate(n))
        , m_delv(allocate(n))
        , m_p(allocate(n))
        , m_q(allocate(n))
        , m_qq(allocate(n))
        , m_ql(allocate(n))
    {}

    ~Domain() {
        free(m_e, n);
        free(m_delv, n);
        free(m_p, n);
        free(m_q, n);
        free(m_qq, n);
        free(m_ql, n);
    }

    T& e(std::size_t index) { return m_e[index]; }
    T& delv(std::size_t index) { return m_delv[index]; }
    T& p(std::size_t index) { return m_p[index]; }
    T& q(std::size_t index) { return m_q[index]; }
    T& qq(std::size_t index) { return m_qq[index]; }
    T& ql(std::size_t index) { return m_ql[index]; }

private:
    T* m_e;
    T* m_delv;
    T* m_p;
    T* m_q;
    T* m_qq;
    T* m_ql;
};

int main() {
    std::vector<double> times;
    std::size_t n_times = 11;
    times.reserve(n_times);

    T* e_old = allocate(n);
    T* delvc = allocate(n);
    T* p_old = allocate(n);
    T* q_old = allocate(n);
    T* qq_old = allocate(n);
    T* ql_old = allocate(n);

    Domain* dptr = reinterpret_cast<Domain*>(sycl::malloc_shared(sizeof(Domain), oneapi::dpl::execution::dpcpp_default.queue()));
    ::new(dptr) Domain();

    for (std::size_t i = 0; i < n_times; ++i) {
        std::vector<T> tmp(n);

        auto start = std::chrono::high_resolution_clock::now();

        std::for_each_n(std::execution::par_unseq, oneapi::dpl::counting_iterator<std::size_t>(0), n,
                    [=](std::size_t i) {
                      e_old[i] = dptr->e(i);
                      delvc[i] = dptr->delv(i);
                      p_old[i] = dptr->p(i);
                      q_old[i] = dptr->q(i);
                      qq_old[i] = dptr->qq(i);
                      ql_old[i] = dptr->ql(i);
                    });

        auto finish = std::chrono::high_resolution_clock::now();
        times.emplace_back(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
    }

    std::cout << "Run completed" << std::endl;
    std::sort(times.begin(), times.end());
    std::cout << "Elapsed time: " << times[n_times / 2] << " ms" << std::endl;

    dptr->~Domain();
    sycl::free(dptr, oneapi::dpl::execution::dpcpp_default.queue());

    free(ql_old, n);
    free(qq_old, n);
    free(q_old, n);
    free(p_old, n);
    free(delvc, n);
    free(e_old, n);
}
