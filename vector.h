#ifndef PHPCXX_VECTOR_H_
#define PHPCXX_VECTOR_H_

#include "phpcxx.h"

#include <vector>
#include "emallocallocator.h"

namespace phpcxx {

template<typename T> using vector = std::vector<T, EMallocAllocator<T> >;

}


#endif /* PHPCXX_VECTOR_H */
