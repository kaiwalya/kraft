#ifndef _INTEGER_TYPES_H_
#define _INTEGER_TYPES_H_


#define MakeTypes(i, sz, type) typedef type i##sz; typedef unsigned type u##i##sz

namespace kq{

namespace core{

MakeTypes(i, 8, __int8);
MakeTypes(i, 16, __int16);
MakeTypes(i, 32, __int32);
MakeTypes(i, 64, __int64);

}

}

#undef MakeTypes

#endif