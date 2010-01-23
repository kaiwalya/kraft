#ifndef _INTEGER_TYPES_H_
#define _INTEGER_TYPES_H_




#if defined(_MSC_VER)

namespace kq{
namespace core{
#define MakeTypes(i, sz, type) typedef type i##sz; typedef unsigned type u##i##sz
MakeTypes(i, 8, __int8);
MakeTypes(i, 16, __int16);
MakeTypes(i, 32, __int32);
MakeTypes(i, 64, __int64);
}
}
#undef MakeTypes

#elif defined(__GNUC__)

#include "stdint.h"
namespace kq{
namespace core{
#define MakeTypes(i, sz, type) typedef type i##sz; typedef u##type u##i##sz
MakeTypes(i, 8, int8_t);
MakeTypes(i, 16, int16_t);
MakeTypes(i, 32, int32_t);
MakeTypes(i, 64, int64_t);
#undef MakeTypes
}
}

#endif



#endif
