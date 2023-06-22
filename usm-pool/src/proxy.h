/*
    Copyright (c) 2005-2021 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _TBB_malloc_proxy_H_
#define _TBB_malloc_proxy_H_

#define MALLOC_UNIXLIKE_OVERLOAD_ENABLED __linux__
#define MALLOC_ZONE_OVERLOAD_ENABLED __APPLE__

// MALLOC_UNIXLIKE_OVERLOAD_ENABLED depends on MALLOC_CHECK_RECURSION stuff
// TODO: limit MALLOC_CHECK_RECURSION to *_OVERLOAD_ENABLED only
#if __unix__ || __APPLE__ || MALLOC_UNIXLIKE_OVERLOAD_ENABLED
#define MALLOC_CHECK_RECURSION 1
#endif

#include <stddef.h>
#include <atomic>

#include <sycl/queue.hpp>
#include <sycl/detail/common.hpp>
#include <sycl/detail/export.hpp>
#include <sycl/ext/intel/experimental/usm_properties.hpp>
#include <sycl/usm/usm_enums.hpp>

using aligned_alloc_shared_type =
    void *(*)(size_t Alignment, size_t Size, const sycl::device &Dev,
                     const sycl::context &Ctxt,
                     const sycl::property_list &PropList, const sycl::detail::code_location &CodeLoc);

using sycl_free_type =
    void (*)(void *ptr, const sycl::context &Ctxt, const sycl::detail::code_location &CodeLoc);

using realloc_type =
    void *(*)(void *ptr, size_t size);

void *safer_aligned_malloc(size_t size, size_t alignment);
void* safer_malloc(size_t size);
void* safer_calloc(size_t nmemb, size_t size);

void   safer_free( void *ptr, void (*original_free)(void*));
void * safer_realloc( void *ptr, size_t, realloc_type, void *original_malloc);
void * safer_aligned_realloc( void *ptr, size_t, size_t, void* );
size_t safer_msize( void *ptr, size_t (*orig_msize_crt80d)(void*));
size_t safer_aligned_msize( void *ptr, size_t, size_t, size_t (*orig_msize_crt80d)(void*,size_t,size_t));

// Struct with original free() and _msize() pointers
struct orig_ptrs {
    void   (*free) (void*);
    size_t (*msize)(void*);
};

struct orig_aligned_ptrs {
    void   (*aligned_free) (void*);
    size_t (*aligned_msize)(void*,size_t,size_t);
};


class RecursiveMallocCallProtector {
#if _WIN32
    using tid = std::thread::id;

    static tid myTid() { return std::this_thread::get_id(); }
#else
    using tid = pthread_t;

    static tid myTid() { return pthread_self(); }
#endif
    // pointer to an automatic data of holding thread
    static std::atomic<void*> autoObjPtr;
    static std::mutex rmc_mutex;
    static std::atomic<tid> owner_thread;
/* Under FreeBSD 8.0 1st call to any pthread function including pthread_self
   leads to pthread initialization, that causes malloc calls. As 1st usage of
   RecursiveMallocCallProtector can be before pthread initialized, pthread calls
   can't be used in 1st instance of RecursiveMallocCallProtector.
   RecursiveMallocCallProtector is used 1st time in checkInitialization(),
   so there is a guarantee that on 2nd usage pthread is initialized.
   No such situation observed with other supported OSes.
 */
#if __FreeBSD__
    static bool        canUsePthread;
#else
    static const bool  canUsePthread = true;
#endif

    char scoped_space;

public:

    RecursiveMallocCallProtector() {
        rmc_mutex.lock();
        if (canUsePthread)
            owner_thread.store(myTid(), std::memory_order_relaxed);
        autoObjPtr.store(&scoped_space, std::memory_order_relaxed);
    }
    ~RecursiveMallocCallProtector() {
        autoObjPtr.store(nullptr, std::memory_order_relaxed);
        rmc_mutex.unlock();
    }
    static bool sameThreadActive() {
        if (!autoObjPtr.load(std::memory_order_relaxed)) // fast path
            return false;
        // Some thread has an active recursive call protector; check if the current one.
        // Exact pthread_self based test
        if (canUsePthread) {
            if (owner_thread.load(std::memory_order_relaxed) == myTid())
                return true;
            else
                return false;
        }
        // inexact stack size based test
        const uintptr_t threadStackSz = 2*1024*1024;
        int dummy;

        uintptr_t xi = (uintptr_t)autoObjPtr.load(std::memory_order_relaxed), yi = (uintptr_t)&dummy;
        uintptr_t diffPtr = xi > yi ? xi - yi : yi - xi;

        return diffPtr < threadStackSz;
    }
};

#if _WIN32
#include <oneapi/tbb/scalable_allocator.h>

inline void *orig_aligned_alloc(size_t size, size_t alignment) {
    return scalable_aligned_malloc(size, alignment);
}
#else
void *orig_aligned_alloc(size_t size, size_t alignment);
#endif

#endif /* _TBB_malloc_proxy_H_ */
