#ifndef TOP_LEVEL_STD
#define TOP_LEVEL_ONEAPI_TBB_VERSION_H
#define TOP_LEVEL_STD
#endif // TOP_LEVEL_STD

#include_next <oneapi/tbb/version.h>

#ifdef TOP_LEVEL_ONEAPI_TBB_VERSION_H
#undef TOP_LEVEL_ONEAPI_TBB_VERSION_H
#undef TOP_LEVEL_STD

#endif // TOP_LEVEL_ONEAPI_TBB_VERSION_H
