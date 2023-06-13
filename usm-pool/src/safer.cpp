
#include <sstream>
#define private public
#include <oneapi/dpl/execution>
#undef private

#define TBB_PREVIEW_MEMORY_POOL 1
#include <oneapi/tbb/memory_pool.h>

// #include <atomic>
// #include <unordered_set>

#include "proxy.h"

std::mutex RecursiveMallocCallProtector::rmc_mutex;
std::atomic<pthread_t> RecursiveMallocCallProtector::owner_thread;
std::atomic<void*> RecursiveMallocCallProtector::autoObjPtr;

static sycl::queue dpcpp_default_queue = oneapi::dpl::execution::dpcpp_default.queue();

static void *call_orig_malloc(size_t size, size_t alignment, void *original_malloc)
{
    return ((void*(*)(size_t))original_malloc)(size);
}

constexpr size_t OFFSET = 64;

void *raw_alloc(std::intptr_t pool_id, std::size_t &bytes)
{
    printf("Call raw_alloc\n");
    return sycl::aligned_alloc_shared(64, bytes, dpcpp_default_queue);
}

int raw_free(std::intptr_t pool_id, void* raw_ptr, std::size_t raw_bytes)
{
    printf("Call raw_free\n");
    sycl::detail::code_location empty;
    // leak memory, if dpcpp_default_queue expired,
    // because have no other choice
    if (dpcpp_default_queue.impl && dpcpp_default_queue.impl.use_count())
        sycl::_V1::free(raw_ptr, dpcpp_default_queue.get_context(), empty);
    return 0;
}

struct MemProvider {
    rml::MemoryPool *pool;

    MemProvider() {
        rml::MemPoolPolicy policy{raw_alloc, raw_free, 0, /*fixedPool=*/false,
            /*keepAllMemory=*/false};
        RecursiveMallocCallProtector scoped;
        rml::MemPoolError err = rml::pool_create_v1(intptr_t(this), &policy, &pool);
        assert(err == rml::MemPoolError::POOL_OK);
    }
};

static MemProvider mem_provider;

void *safer_aligned_malloc(size_t size, size_t alignment, void *original_malloc)
{
    if (RecursiveMallocCallProtector::sameThreadActive()) {
        void *res = call_orig_malloc(size + OFFSET, alignment, original_malloc);
        if (!res)
            return nullptr;

        *(size_t*)res = 1;
        ((size_t*)res)[1] = size;
        return (char*)res + OFFSET;
    }
    RecursiveMallocCallProtector scoped;
    bool use_original = !(dpcpp_default_queue.impl
        && dpcpp_default_queue.impl.use_count());

    void *res = use_original?
            call_orig_malloc(size + OFFSET, alignment, original_malloc)
        : rml::pool_aligned_malloc(mem_provider.pool, size + OFFSET, alignment);
    if (!res)
        return nullptr;

    *(size_t*)res = use_original;
    ((size_t*)res)[1] = size;
    return (char*)res + OFFSET;
}

void safer_free(void *user_ptr, void (*original_free)(void*), sycl_free_type orig_sycl_free)
{
    if (!user_ptr)
        return;
    void *ptr = (char*)user_ptr - OFFSET;
    bool use_original_free = *(size_t*)ptr;
    [[maybe_unused]] size_t size = ((size_t*)ptr)[1];

    if (use_original_free) {
        original_free(ptr);
    } else {
        rml::pool_free(mem_provider.pool, ptr);
    }
}

void *safer_realloc(void *user_ptr, size_t size, realloc_type orig_realloc, void *original_malloc)
{
    if (!user_ptr)
        return safer_aligned_malloc(size, sizeof(void*), original_malloc);

    void *ptr = (char*)user_ptr - OFFSET;
    bool use_original = *(size_t*)ptr;
    if (use_original) {
        if (!size)
            return orig_realloc(ptr, 0);
        void *new_ptr = orig_realloc(ptr, size + OFFSET);
        if (!new_ptr)
            return nullptr;
        *(size_t*)new_ptr = 1;
        ((size_t*)new_ptr)[1] = size;
        return (char*)new_ptr + OFFSET;
    }
    if (!size) {
        rml::pool_free(mem_provider.pool, ptr);
        return nullptr;
    }
    void *new_ptr = pool_realloc(mem_provider.pool, ptr, size + OFFSET);
    if (!new_ptr)
        return nullptr;
    *(size_t*)new_ptr = 0;
    ((size_t*)new_ptr)[1] = size;
    return (char*)new_ptr + OFFSET;
}

void * safer_aligned_realloc( void *ptr, size_t, size_t, void* )
{
    return nullptr;
}

size_t safer_msize( void *ptr, size_t (*orig_msize_crt80d)(void*))
{
    void *header = (char*)ptr - OFFSET;
    return ((size_t*)header)[1];
}

size_t safer_aligned_msize( void *ptr, size_t, size_t, size_t (*orig_msize_crt80d)(void*,size_t,size_t))
{
    return 0;
}
