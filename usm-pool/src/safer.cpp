
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
std::atomic<RecursiveMallocCallProtector::tid> RecursiveMallocCallProtector::owner_thread;
std::atomic<void*> RecursiveMallocCallProtector::autoObjPtr;

static const size_t UNIQ_TYPE_CONST = 0x23499abc405a9bccLLU;

struct BlockHeader {
    size_t use_orig;
    size_t size;
    size_t type;
};

constexpr size_t HEADER_OFFSET = 64;

static_assert(sizeof(BlockHeader) <= HEADER_OFFSET);

void *raw_alloc(std::intptr_t pool_id, std::size_t &bytes)
{
    sycl::queue dpcpp_default_queue = oneapi::dpl::execution::dpcpp_default.queue();

    return sycl::aligned_alloc_shared(64, bytes, dpcpp_default_queue);
}

int raw_free(std::intptr_t pool_id, void* raw_ptr, std::size_t raw_bytes)
{
    // leak memory, if dpcpp_default_queue expired,
    // because have no other choice
    if (bool(oneapi::dpl::execution::dpcpp_default)) {
        sycl::queue dpcpp_default_queue = oneapi::dpl::execution::dpcpp_default.queue();
        sycl::detail::code_location empty;
        if (dpcpp_default_queue.impl && dpcpp_default_queue.impl.use_count())
            sycl::_V1::free(raw_ptr, dpcpp_default_queue.get_context(), empty);
    }
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

void *safer_malloc(size_t size)
{
    return safer_aligned_malloc(size, sizeof(void*));
}

void *safer_calloc(size_t nmemb, size_t size)
{
    void* ptr = safer_malloc(nmemb * size);
    return ptr ? memset(ptr, 0, nmemb * size) : nullptr;
}

void *safer_aligned_malloc(size_t size, size_t alignment)
{
    if (RecursiveMallocCallProtector::sameThreadActive()) {
        void *raw = orig_aligned_alloc(size + HEADER_OFFSET, alignment);
        if (!raw)
            return nullptr;

        BlockHeader* header = (BlockHeader*)raw;
        header->use_orig = 1;
        header->size = size;
        header->type = UNIQ_TYPE_CONST;
        void *res = (char*)raw + HEADER_OFFSET;
        assert((uintptr_t)res % alignment == 0);
        return res;
    }

    RecursiveMallocCallProtector scoped;
#if 1
    bool use_original = ! bool(oneapi::dpl::execution::dpcpp_default);
#else
    sycl::queue dpcpp_default_queue = oneapi::dpl::execution::dpcpp_default.queue();
    bool use_original = !(dpcpp_default_queue.impl
        && dpcpp_default_queue.impl.use_count());
#endif
    
    void *raw = use_original?
            orig_aligned_alloc(size + HEADER_OFFSET, alignment)
        : rml::pool_aligned_malloc(mem_provider.pool, size + HEADER_OFFSET, alignment);
    if (!raw)
        return nullptr;

    BlockHeader* header = (BlockHeader*)raw;
    header->use_orig = use_original;
    header->size = size;
    header->type = UNIQ_TYPE_CONST;
    void *res = (char*)raw + HEADER_OFFSET;
    assert((uintptr_t)res % alignment == 0);
    return res;
}

void safer_free(void *user_ptr, void (*original_free)(void*))
{
    if (!user_ptr)
        return;
    void *ptr = (char*)user_ptr - HEADER_OFFSET;
    BlockHeader* header = (BlockHeader*)ptr;
    bool use_original_free = header->use_orig;
    [[maybe_unused]] size_t size = header->size;

#if _WIN32
    if (header->type != UNIQ_TYPE_CONST)
        original_free(user_ptr);
    else if (use_original_free)
        scalable_free(ptr);
#else
    assert(header->type == UNIQ_TYPE_CONST);
    if (use_original_free)
        original_free(ptr);
#endif
    else
        rml::pool_free(mem_provider.pool, ptr);
}

void *safer_realloc(void *user_ptr, size_t size, realloc_type orig_realloc, void *original_malloc)
{
    if (!user_ptr)
        return safer_aligned_malloc(size, sizeof(void*));

    void *ptr = (char*)user_ptr - HEADER_OFFSET;
    BlockHeader* header = (BlockHeader*)ptr;
    bool use_original = header->use_orig;
    if (use_original) {
        if (!size)
            return orig_realloc(ptr, 0);
        void *new_ptr = orig_realloc(ptr, size + HEADER_OFFSET);
        if (!new_ptr)
            return nullptr;
        BlockHeader* header = (BlockHeader*)new_ptr;
        header->use_orig = 1;
        header->size = size;
        header->type = UNIQ_TYPE_CONST;

        return (char*)new_ptr + HEADER_OFFSET;
    }
    if (!size) {
        rml::pool_free(mem_provider.pool, ptr);
        return nullptr;
    }
    void *new_ptr = pool_realloc(mem_provider.pool, ptr, size + HEADER_OFFSET);
    if (!new_ptr)
        return nullptr;
    header = (BlockHeader*)new_ptr;
    header->use_orig = 0;
    header->size = size;
    header->type = UNIQ_TYPE_CONST;

    return (char*)new_ptr + HEADER_OFFSET;
}

void * safer_aligned_realloc( void *ptr, size_t, size_t, void* )
{
    return nullptr;
}

size_t safer_msize( void *ptr, size_t (*orig_msize_crt80d)(void*))
{
    BlockHeader* header = (BlockHeader*)((char*)ptr - HEADER_OFFSET);
    assert((header->type == UNIQ_TYPE_CONST));
    return header->size;
}

size_t safer_aligned_msize( void *ptr, size_t, size_t, size_t (*orig_msize_crt80d)(void*,size_t,size_t))
{
    return 0;
}
